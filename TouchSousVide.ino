#include <Adafruit_GFX_AS8.h>    // Core graphics library
#include <Adafruit_ILI9341_AS8.h> // Hardware-specific library
#include <TouchScreen.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "recipies.h"

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
// Status pixel positions
#define CURRENT_TEMPERATURE_X (sizeof(CURRENT_TEMPERATURE_LABEL) * CHARACTER_PIXEL_WIDTH)
#define CURRENT_TEMPERATURE_Y (0 * CHARACTER_PIXEL_HEIGHT)
#define TARGET_TEMPERATURE_X (sizeof(TARGET_TEMPERATURE_LABEL) * CHARACTER_PIXEL_WIDTH)
#define TARGET_TEMPERATURE_Y (1 * CHARACTER_PIXEL_HEIGHT)
#define RECIPE_DETAILS_X 0
#define RECIPE_DETAILS_Y (4 * CHARACTER_PIXEL_HEIGHT)
#define RECIPE_DETAILS_HEIGHT (4 * CHARACTER_PIXEL_HEIGHT)
#define STOPWATCH_X 0
#define STOPWATCH_Y (8 * CHARACTER_PIXEL_HEIGHT)

// Actual values
double current_temperature, target_temperature;
// Last rendered values
double last_current_temperature, last_target_temperature;

// Timestamps for the next time to perform an action
unsigned long next_lcd_update, next_temperature_read, next_touch_read, next_buzzer_check;

// Timestamp for stopwatch
unsigned long start_time;
// Last render time for stopwatch
unsigned long last_stopwatch_time;

#define MENU_NOT_CHOSEN -1
int menu_animal_idx, menu_type_idx;
// Last rendered menu values
int last_menu_animal_idx, last_menu_type_idx;

const Recipe* current_recipe;
// Last rendered recipe
const Recipe* last_current_recipe;

// Polling periods in milliseconds
#define LCD_UPDATE_INTERVAL 250
#define TEMPERATURE_READ_INTERVAL 1000
#define TOUCH_READ_INTERVAL 125
#define DELAYED_TOUCH_READ_INTERVAL 500
#define BUZZER_CHECK_INTERVAL 100

// Number of degrees over target before warning state is triggered
#define OVERHEATING_THRESHOLD 0.5


void renderButton(int idx, const char* text, bool clear = true) {
  // Renders the text for a button to the screen. The buttons are laid out in a grid:
  //
  //  0  1  2
  //  3  4  5
  //  6  7  8
  //  9 10 11
  const int16_t x_col = idx % 3;
  const int16_t y_row = idx / 3;
  const int16_t x = (BUTTON_WIDTH * x_col) + (BUTTON_WIDTH / 2) - ((strlen(text) * CHARACTER_PIXEL_WIDTH) / 2);
  const int16_t y = (BUTTON_HEIGHT * y_row) + (BUTTON_HEIGHT / 2) - (CHARACTER_PIXEL_HEIGHT / 2);
  if (clear) {
    // + 1 to not paint over the border line
    const int16_t start_x = (BUTTON_WIDTH * x_col) + 1;
    // -2 on width to avoid the border line
    tft.fillRect(start_x, y, BUTTON_WIDTH - 2, CHARACTER_PIXEL_HEIGHT, ILI9341_BLACK);
  }
  tft.setCursor(x, y);
  tft.print(text);
}

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
  menu_animal_idx = MENU_NOT_CHOSEN;
  menu_type_idx = MENU_NOT_CHOSEN;
  start_time = millis();

  
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
  renderButton(2,  "+ 1.0");
  renderButton(5,  "+ 0.1");
  renderButton(8,  "- 0.1");
  renderButton(11, "- 1.0");
  
  // Render the timer restart button
  renderButton(9, "Restart");
}

void renderMenu() {
  if (menu_animal_idx == MENU_NOT_CHOSEN) {
    renderButton(3, animals[0].animal);
    renderButton(4, animals[1].animal);
    renderButton(6, animals[2].animal);
    renderButton(7, animals[3].animal);
  } else if (menu_type_idx == MENU_NOT_CHOSEN) {
    const Animal* animal = &animals[menu_animal_idx];
    renderButton(3, animal->types[0].type);
    renderButton(4, animal->types[1].type);
    renderButton(6, animal->types[2].type);
    renderButton(7, "<Back>");
  } else {
    const Animal* animal = &animals[menu_animal_idx];
    const AnimalTypes* animalType = &animal->types[menu_type_idx];
    renderButton(3, animalType->recipies[0].level);
    renderButton(4, animalType->recipies[1].level);
    renderButton(6, animalType->recipies[2].level);
    renderButton(7, "<Back>");
  }
}

void writeTime(unsigned long minutes) {
  unsigned long hours = minutes / 60;
  if (hours < 10) {
    tft.print(" ");
  }
  tft.print(hours);
  tft.print(":");
  if ((minutes % 60) < 10) {
    tft.print("0");
  }
  tft.print(minutes % 60);
}

void renderRecipe() {
  // Clear any pre-existing recipe
  tft.fillRect(RECIPE_DETAILS_X, RECIPE_DETAILS_Y, BUTTON_WIDTH * 2, RECIPE_DETAILS_HEIGHT, ILI9341_BLACK);
  if (current_recipe) {
    const Animal* animal = &animals[menu_animal_idx];
    const AnimalTypes* animalType = &animal->types[menu_type_idx];
    // Render the recipe name
    tft.setCursor(RECIPE_DETAILS_X, RECIPE_DETAILS_Y);
    tft.print(animal->animal);
    tft.print(", ");
    tft.println(animalType->type);
    tft.print(" - ");
    tft.println(current_recipe->level);
    writeTime(current_recipe->ideal_minutes);
    tft.println(" (Best)");
    writeTime(current_recipe->last_call_minutes);
    tft.println(" (Last Call)");
  }
}

void renderStopwatch() {
 tft.fillRect(STOPWATCH_X, STOPWATCH_Y, BUTTON_WIDTH * 2, CHARACTER_PIXEL_HEIGHT, ILI9341_BLACK);
 tft.setCursor(STOPWATCH_X, STOPWATCH_Y);
 unsigned long elapsed = millis() - start_time;
 writeTime(elapsed / 60000);
 tft.println(" (Duration)");
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
  if ((menu_animal_idx != last_menu_animal_idx) ||
      (menu_type_idx != last_menu_type_idx)) {
    renderMenu();
    last_menu_animal_idx = menu_animal_idx;
    last_menu_type_idx = menu_type_idx;
  }
  if (current_recipe != last_current_recipe) {
    renderRecipe();
    last_current_recipe = current_recipe; 
  }
  if (last_stopwatch_time + 60000 < millis()) {
    renderStopwatch();
    last_stopwatch_time = millis();
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

void screenTouched() {
  // Delay the next scan so that double clicks don't happen.
  next_touch_read = millis() + DELAYED_TOUCH_READ_INTERVAL;
}

void adjustTarget(double delta) {
  target_temperature += delta;
  if (delta != 0) {
    screenTouched();
  }
}

void setRecipe(const Recipe* recipe) {
  target_temperature = recipe->temperature;
  current_recipe = recipe;
}

void menuPress(int idx) {
  // The menu button layout is as follows
  //        +1.0
  //  0  1  +0.1
  //  2  3  -0.1
  //  4  5  +1.0
  if (idx < 4) {
    // A selection on the food type menu
    if (menu_animal_idx == MENU_NOT_CHOSEN) {
      menu_animal_idx = idx;
    } else if (menu_type_idx == MENU_NOT_CHOSEN) {
      if (idx == 3) {
        // Back button
        menu_animal_idx = MENU_NOT_CHOSEN;
      } else {
        if (animals[menu_animal_idx].types[idx].type[0] != 0) {
          // Only set non-empty entries
          menu_type_idx = idx;
        }
      }
    } else {
      if (idx == 3) {
        // Back button
        menu_type_idx = MENU_NOT_CHOSEN;
      } else {
        const Animal* animal = &animals[menu_animal_idx];
        const AnimalTypes* animalType = &animal->types[menu_type_idx];
        const Recipe* recipe = &animalType->recipies[idx];
        if (recipe->level[0] != 0) {
          // Only set non-empty entries
          setRecipe(recipe);
        }
      }
    }
    screenTouched();
  } else if (idx == 4) {
    // Restart timer button
    start_time = millis();
    renderStopwatch();  
  }
}

void readTouch() {
  TSPoint current_touch = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  // scale from 0->1023 to tft.width
  current_touch.x = map(current_touch.x, TS_MINX, TS_MAXX, tft.width(), 0);
  current_touch.y = map(current_touch.y, TS_MINY, TS_MAXY, tft.height(), 0);
  if (current_touch.z >= ts.pressureThreshhold) {
    // Work out the button idx
    const int x_col = current_touch.x / BUTTON_WIDTH;
    const int y_row = current_touch.y / BUTTON_HEIGHT;
    const int button_idx = x_col + (3 * y_row);
    // Handle the button
    switch(button_idx) {
      // Temperature buttons
      case 2:  adjustTarget( 1.0); break;
      case 5:  adjustTarget( 0.1); break;
      case 8:  adjustTarget(-0.1); break;
      case 11: adjustTarget(-1.0); break;
      // Menu buttons
      case 3:   menuPress(0); break;
      case 4:   menuPress(1); break;
      case 6:   menuPress(2); break;
      case 7:   menuPress(3); break;
      case 9:   menuPress(4); break;
      case 10:  menuPress(5); break;
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
