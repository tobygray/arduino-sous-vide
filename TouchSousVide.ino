#include <Adafruit_GFX_AS8.h>    // Core graphics library
#include <Adafruit_ILI9341_AS8.h> // Hardware-specific library
#include <OneWire.h>
#include <DallasTemperature.h>

// ************ PIN CONFIG ************
// 433Mhz Transmitter data pin
#define TRANSMIT_PIN 11
// Data wire pin
#define ONE_WIRE_BUS 12
// Relay trigger pin
#define TRIGGER_PIN 13
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

// Setup a oneWire instance
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


const char CURRENT_TEMPERATURE_LABEL[] = "Current temperature:";
const char TARGET_TEMPERATURE_LABEL[]  = "Target temperature :";

// Pixel layout definitions
#define CHARACTER_PIXEL_WIDTH 6
#define CHARACTER_PIXEL_HEIGHT 8
#define CURRENT_TEMPERATURE_X (sizeof(CURRENT_TEMPERATURE_LABEL) * CHARACTER_PIXEL_WIDTH)
#define CURRENT_TEMPERATURE_Y (0 * CHARACTER_PIXEL_HEIGHT)
#define TARGET_TEMPERATURE_X (sizeof(TARGET_TEMPERATURE_LABEL) * CHARACTER_PIXEL_WIDTH)
#define TARGET_TEMPERATURE_Y (1 * CHARACTER_PIXEL_HEIGHT)


double current_temperature, target_temperature;

void setup() {
  // Screen config
  tft.reset();
  delay(10);
  tft.begin(0x9341);
  tft.setRotation(3);
  tft.fillScreen(ILI9341_GREY);
  tft.setTextColor(ILI9341_WHITE, ILI9341_GREY);
  
  // Temperature sensor config
  sensors.begin();
  
  // Set some initial values
  current_temperature = 20.0;
  target_temperature = 58; // Aim for beef
  
  // Draw the initial screen
  tft.println(CURRENT_TEMPERATURE_LABEL);
  tft.println(TARGET_TEMPERATURE_LABEL);
}

void loop() {
  sensors.requestTemperatures(); // Send the command to get temperatures  
  current_temperature = sensors.getTempCByIndex(0);
  tft.setCursor(CURRENT_TEMPERATURE_X, CURRENT_TEMPERATURE_Y);
  tft.println(current_temperature);
  tft.setCursor(TARGET_TEMPERATURE_X, TARGET_TEMPERATURE_Y);
  tft.println(target_temperature);
  

}
