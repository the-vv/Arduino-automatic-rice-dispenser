#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "HX711.h"

// PARAMETERS
#define calibration_factor -7050.0 // This value is obtained using the SparkFun_HX711_Calibration sketch
#define DISPENSER_WEIGHT 5         // This is the weight of the rice that will be dispensed in kg
#define INITIAL_LOADING_DELAY 5000
#define AUTO_MODE_EXCHANGE_DELAY 20000 //

// // initialize the liquid crystal library
// // the first parameter is  the I2C address
// // the second parameter is how many rows are on your screen
// // the  third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;

#define LOADCELL_DOUT_PIN 3
#define LOADCELL_SCK_PIN 2

// Switches
#define AUTO_MODE_SWITCH 8         // if this is on, the system will automatically dispense the rice with preset weight
#define MANUAL_MODE_SWITCH 9       // if this is on, system will just act like a weighing machine
#define PLUS_SWITCH 10             // this will increase the weight in manual dispenser mode
#define MINUS_SWITCH 11            // this will decrease the weight in manual dispenser mode
#define MANUAL_DISPENSER_SWITCH 12 // this will turn on the manual dispenser mode

// Variables
float currentWeight = 0.0;
String currentMode = "AUTO"; // WEIGHTING, AUTO, MANUAL
unsigned long motorStartTime = 0;
bool waitingForExchange = false;
unsigned long exchangeStartTime = 0;
bool isValueOpen = false;
String lastDisplayText = "";


float getWeight();
void open();
void close();
void processAutoMode(float weight);
void showInDisplay(String, String);

void setup()
{
  // Setup Pins

  pinMode(AUTO_MODE_SWITCH, INPUT_PULLUP);
  pinMode(MANUAL_MODE_SWITCH, INPUT_PULLUP);
  pinMode(PLUS_SWITCH, INPUT_PULLUP);
  pinMode(MINUS_SWITCH, INPUT_PULLUP);
  pinMode(MANUAL_DISPENSER_SWITCH, INPUT_PULLUP);

  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  // Setup Weighting Scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); // This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();                        // Assuming there is no weight on the scale at start up, reset the scale to 0

  // Startup
  lcd.setCursor(0, 0);
  lcd.print("Please Load Rice");
  lcd.setCursor(0, 1);
  lcd.print("Upto ");
  lcd.print(DISPENSER_WEIGHT);
  lcd.print(" kg");
  // delay for 2 seconds
  delay(INITIAL_LOADING_DELAY);
  motorStartTime = millis();
  open();
  lcd.clear();
}
void loop()
{
  float weight = getWeight();

  if (!waitingForExchange)
  {
    // lcd section
    lcd.setCursor(0, 0);
    lcd.print("Mode: ");
    lcd.print(currentMode);
    lcd.setCursor(0, 1);
    lcd.print("Weight: ");
    lcd.print(weight);
  } else if (currentMode == "AUTO") {
    if ((millis() - exchangeStartTime) < (AUTO_MODE_EXCHANGE_DELAY / 2)) {
      showInDisplay("Secure your", "Dispenser");
    } else {
      showInDisplay("Waiting For", "Next Dispenser");
    }
  }

  if (currentMode == "AUTO")
  {
    processAutoMode(weight);
  }
}

float getWeight()
{
  float scaleLbs = scale.get_units();
  float scaleKg = scaleLbs * 0.453592;
  return -scaleKg;
}

// Auto Mode Functionality
void processAutoMode(float weight)
{
  if (waitingForExchange)
  {
    if ((millis() - exchangeStartTime) > AUTO_MODE_EXCHANGE_DELAY)
    {
      open();
      waitingForExchange = false;
      isValueOpen = true;
      lcd.clear();
    }
  }
  if (weight >= DISPENSER_WEIGHT)
  {
    close();
    isValueOpen = false;
    waitingForExchange = true;
    exchangeStartTime = millis();
  }
}

void open()
{
  Serial.println("Opening Dispenser");
}

void close()
{
  Serial.println("Closing Dispenser");
}

void showInDisplay(String line1, String line2)
{
  if (lastDisplayText == line1 + line2)
  {
    return;
  }
  lcd.clear();
  lastDisplayText = line1 + line2;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}