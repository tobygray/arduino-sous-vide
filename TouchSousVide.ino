#include <Adafruit_GFX_AS8.h>    // Core graphics library
#include <Adafruit_ILI9341_AS8.h> // Hardware-specific library
#include <TouchScreen.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ************ PIN CONFIG ************
// 433Mhz Transmitter data pin
#define TRANSMIT_PIN 11
// Data wire pin
#define ONE_WIRE_BUS 12
// Relay trigger pin
#define TRIGGER_PIN 13
// Buzzer trigger pin
#define BUZZER_PIN 10
// ************ END PIN CONFIG ************

// ***** START SCREEN SETUP *****
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

Adafruit_ILI9341_AS8 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_ILI9341_AS8 tft;

#define ILI9341_GREY 0x5AEB
// ***** END SCREEN SETUP *****

// ***** START TOUCHSCREEN SETUP *****
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
#define YP A2 
#define XM A1 
#define YM 6 
#define XP 7 
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
// ***** END TOUCHSCREEN SETUP *****

// Setup a oneWire instance
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress temperature_address;


const char CURRENT_TEMPERATURE_LABEL[] = "Current temperature:";
const char TARGET_TEMPERATURE_LABEL[]  = "Target temperature :";

// Pixel layout definitions
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT BUTTON_WIDTH
#define CHARACTER_PIXEL_WIDTH 6
#define CHARACTER_PIXEL_HEIGHT 8
#define CURRENT_TEMPERATURE_X (sizeof(CURRENT_TEMPERATURE_LABEL) * CHARACTER_PIXEL_WIDTH)
#define CURRENT_TEMPERATURE_Y (0 * CHARACTER_PIXEL_HEIGHT)
#define TARGET_TEMPERATURE_X (sizeof(TARGET_TEMPERATURE_LABEL) * CHARACTER_PIXEL_WIDTH)
#define TARGET_TEMPERATURE_Y (1 * CHARACTER_PIXEL_HEIGHT)

// Actual values
double current_temperature, target_temperature;
// Last rendered values
double last_current_temperature, last_target_temperature;

// Timestamps for the next time to perform an action
unsigned long next_lcd_update, next_temperature_read, next_touch_read, next_buzzer_check;

// Polling periods in milliseconds
#define LCD_UPDATE_INTERVAL 250
#define TEMPERATURE_READ_INTERVAL 1000
#define TOUCH_READ_INTERVAL 125
#define DELAYED_TOUCH_READ_INTERVAL 500
#define BUZZER_CHECK_INTERVAL 100

// Number of degrees over target before warning state is triggered
#define OVERHEATING_THRESHOLD 0.5

// Recommendations from ChefSteps:
//   http://www.chefsteps.com/activities/sous-vide-time-and-temperature-guide
struct Recipe {
  const char level[12] PROGMEM;
  const double temperature;
  const unsigned long ideal_minutes;
  const unsigned long last_call_minutes;
};

struct AnimalTypes {
  const char type[12] PROGMEM;
  const Recipe recipies[3] PROGMEM;
};

struct Animal {
  const char animal[12] PROGMEM;
  const AnimalTypes types[3] PROGMEM;
};

const Animal animals[] = {
  {"Beef", {
    {"Steak", {
      {"Rare", 54, 90, 180},
      {"Medium-R", 58, 90, 180},
      {"Well-D", 70, 90, 180}}},
    {"Roast", {
      {"Rare", 56, 420, 960},
      {"Medium-R", 60, 360, 840},
      {"Well-D", 70, 300, 660}}},
    {"Tough", {
      {"Rare", 58, 1440, 2880},
      {"Medium-R", 65, 960, 1440},
      {"Well-D", 85, 480, 960}}}
    }
  },
};

void setup() {
  // Buzzer config 
  pinMode(BUZZER_PIN, OUTPUT);
 
  // Screen config
  tft.reset();
  delay(10);
  tft.begin(0x9341);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  
  // Temperature sensor config
  sensors.begin();
  sensors.setWaitForConversion(false);
  sensors.setResolution(TEMP_9_BIT);
  sensors.getAddress(temperature_address, 0);
  
  // Set some initial values
  current_temperature = 20.0;
  target_temperature = 58; // Aim for beef
  
  // Draw the initial screen
  tft.println(CURRENT_TEMPERATURE_LABEL);
  tft.println(TARGET_TEMPERATURE_LABEL);
  // Render the borders for the + and - change buttons and menu selection
  tft.drawFastVLine(SCREEN_WIDTH - BUTTON_WIDTH, 0, SCREEN_HEIGHT, ILI9341_WHITE);
  tft.drawFastVLine(SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT, ILI9341_WHITE);
  tft.drawFastHLine(SCREEN_WIDTH - BUTTON_WIDTH, 0, BUTTON_WIDTH, ILI9341_WHITE);
  tft.drawFastHLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, ILI9341_WHITE);
  tft.drawFastVLine(0, BUTTON_HEIGHT, SCREEN_HEIGHT - BUTTON_HEIGHT, ILI9341_WHITE);
  tft.drawFastVLine(BUTTON_WIDTH, BUTTON_HEIGHT, SCREEN_HEIGHT - BUTTON_HEIGHT, ILI9341_WHITE);
  for (int i = 1; i < 4; ++i) {
    tft.drawFastHLine(0, (SCREEN_HEIGHT * i)/ 4, SCREEN_WIDTH, ILI9341_WHITE);
  }
  // Render the text for the +/- and 1.0/0.1 regions
  for (int i = 0; i < 4; ++i) {
    // Render in the center of each button
    const int16_t x = (SCREEN_WIDTH - BUTTON_WIDTH) + (BUTTON_WIDTH / 2) - ((5 * CHARACTER_PIXEL_WIDTH) / 2);
    const int16_t y = (BUTTON_WIDTH * i) + (BUTTON_WIDTH / 2) - (CHARACTER_PIXEL_HEIGHT / 2);
    tft.setCursor(x, y);
    switch(i) {
      case 0: tft.print("+ 1.0"); break;
      case 1: tft.print("+ 0.1"); break;
      case 2: tft.print("- 0.1"); break;
      case 3: tft.print("- 1.0"); break;
    }
  }
}

void updateLCD() {
  if (last_current_temperature != current_temperature) {
    tft.setCursor(CURRENT_TEMPERATURE_X, CURRENT_TEMPERATURE_Y);
    tft.print(current_temperature);
    last_current_temperature = current_temperature;
  }
  if (last_target_temperature != target_temperature) {
    tft.setCursor(TARGET_TEMPERATURE_X, TARGET_TEMPERATURE_Y);
    tft.print(target_temperature);
    last_target_temperature = target_temperature;
  }
}

void readTemperature() {
  sensors.requestTemperatures(); // Send the command to get temperatures  
  current_temperature = sensors.getTempC(temperature_address);
}

void buzzerCheck() {
  // Warn if overheating
  if (current_temperature > target_temperature + OVERHEATING_THRESHOLD) {
    digitalWrite(BUZZER_PIN, HIGH);
    return;
  }
  digitalWrite(BUZZER_PIN, LOW);
}

void readTouch() {
  TSPoint current_touch = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  // scale from 0->1023 to tft.width
  current_touch.x = map(current_touch.x, TS_MINX, TS_MAXX, tft.width(), 0);
  current_touch.y = map(current_touch.y, TS_MINY, TS_MAXY, tft.height(), 0);
  if (current_touch.z >= ts.pressureThreshhold) {
    // Check for temperature adjustment
    double target_delta = 0.0;
    if (current_touch.x >= (SCREEN_WIDTH - BUTTON_WIDTH)) {
      const int button_idx = current_touch.y / BUTTON_HEIGHT;
      switch (button_idx) {
        case 0: target_delta =  1.0; break;
        case 1: target_delta =  0.1; break;
        case 2: target_delta = -0.1; break;
        case 3: target_delta = -1.0; break;
      }
    }
    target_temperature += target_delta;
    if (target_delta != 0) {
      next_touch_read = millis() + DELAYED_TOUCH_READ_INTERVAL;
    }
  }
}

void loop() {
  unsigned long now = millis();
  if (now > next_touch_read) {
    next_touch_read = now + TOUCH_READ_INTERVAL;
    readTouch();
  }
  if (now > next_temperature_read) {
    next_temperature_read = now + TEMPERATURE_READ_INTERVAL;
    readTemperature();
  }
  if (now > next_buzzer_check) {
    next_buzzer_check = now + BUZZER_CHECK_INTERVAL;
    buzzerCheck();
  }
  // Always render last as it can take ages
  if (now > next_lcd_update) {
    next_lcd_update = now + LCD_UPDATE_INTERVAL;
    updateLCD();
  }
}
