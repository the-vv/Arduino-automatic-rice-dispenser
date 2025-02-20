#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "HX711.h"
#include <Valve.h>
#include <Automatic.h>
#include <PressButton.h>

// PARAMETERS
#define calibration_factor -7050.0 // This value is obtained using the SparkFun_HX711_Calibration sketch
#define DISPENSER_WEIGHT 2         // This is the weight of the rice that will be dispensed in kg
#define INITIAL_LOADING_DELAY 5000
#define AUTO_MODE_EXCHANGE_DELAY 20000 //

#define LOADCELL_DOUT_PIN 3
#define LOADCELL_SCK_PIN 2

// Switches
#define AUTO_MODE_SWITCH 8         // if this is on, the system will automatically dispense the rice with preset weight
#define MANUAL_MODE_SWITCH 9       // if this is on, system will just act like a weighing machine
#define PLUS_SWITCH 10             // this will increase the weight in manual dispenser mode
#define MINUS_SWITCH 11            // this will decrease the weight in manual dispenser mode
#define MANUAL_DISPENSER_SWITCH 12 // this will turn on the manual dispenser mode
#define VALVE_PIN 13               // this is the pin for the valve

// initialize the liquid crystal library
// the first parameter is  the I2C address
// the second parameter is how many rows are on your screen
// the  third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;
Valve valve(VALVE_PIN);
Automatic automatic(VALVE_PIN, DISPENSER_WEIGHT, AUTO_MODE_EXCHANGE_DELAY);
PressButton manualDispenserSwitch(MANUAL_DISPENSER_SWITCH); // Create a button instance
PressButton plusSwitch(PLUS_SWITCH);                        // Create a button instance
PressButton minusSwitch(MINUS_SWITCH);                      // Create a button instance

enum Mode
{
    AUTO,
    WEIGHTING,
    MANUAL_SET,
    MANUAL_DISPENSE
};

// Variables
float currentWeight = 0.0;
Mode currentMode = AUTO;
String lastDisplayText = "";
int manualDispenserCustomWeight = 0;

float getWeight();
void showInDisplay(String, String);
String getModeName(Mode mode);

void setup()
{
    // Setup Pins

    pinMode(AUTO_MODE_SWITCH, INPUT_PULLUP);
    pinMode(MANUAL_MODE_SWITCH, INPUT_PULLUP);
    // pinMode(PLUS_SWITCH, INPUT_PULLUP);
    // pinMode(MINUS_SWITCH, INPUT_PULLUP);
    // pinMode(MANUAL_DISPENSER_SWITCH, INPUT_PULLUP);

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
    // delay for few seconds
    delay(INITIAL_LOADING_DELAY);
    valve.open();
    lcd.clear();
}
void loop()
{
    float weight = getWeight();
    if (currentMode == AUTO || currentMode == WEIGHTING)
    {
        showInDisplay("Mode: " + getModeName(currentMode), "Weight: " + String(weight));
    }

    // Check for mode changes and update the current mode
    Mode lastMode = currentMode;
    if (currentMode != MANUAL_SET && currentMode != MANUAL_DISPENSE)
    {
        if (digitalRead(AUTO_MODE_SWITCH) == LOW) // check if the auto mode toggle is enabled.
        {
            currentMode = AUTO;
        }
        else // otherwise, switch to weighting mode
        {
            currentMode = WEIGHTING;
        }
    }
    else if (manualDispenserSwitch.isPressed())
    {
        // If the manual dispenser switch is pressed, switch to manual set mode
        // If the current mode is already manual set, switch to manual dispense mode
        // If the current mode is manual dispense, switch back to auto mode
        if (currentMode == AUTO || currentMode == WEIGHTING)
        {
            currentMode = MANUAL_SET;
        }
        else if (currentMode == MANUAL_SET)
        {
            currentMode = MANUAL_DISPENSE;
        }
        else
        {
            currentMode = AUTO;
        }
    }
    if (lastMode != currentMode)
    {
        valve.close();
        automatic.reset();
        manualDispenserCustomWeight = 0;
    }

    // Act based on the current mode
    if (currentMode == AUTO)
    {
        automatic.processAutoMode(weight);
    }
    else if (currentMode == WEIGHTING)
    {
        // Do nothing, because we are already displaying the weight
    }
    else if (currentMode == MANUAL_SET)
    {
        if (plusSwitch.isPressed())
        {
            manualDispenserCustomWeight += 1;
        }
        else if (minusSwitch.isPressed())
        {
            manualDispenserCustomWeight -= 1;
        }
        showInDisplay("Set Weight: " + manualDispenserCustomWeight, "Press + or -");
    }
    else if (currentMode == MANUAL_DISPENSE)
    {
        showInDisplay("Mode: " + getModeName(currentMode), "Weight: " + String(weight) + "/" + manualDispenserCustomWeight);
        if (weight >= manualDispenserCustomWeight)
        {
            valve.close();
            currentMode= MANUAL_SET;
            showInDisplay("Switched to " + getModeName(currentMode), "Press Dispense");
            Serial.println("Switched to " + getModeName(currentMode) + "Because weight is greater than set weight");
        }
    }
}

float getWeight()
{
    float scaleLbs = scale.get_units();
    float scaleKg = scaleLbs * 0.453592;
    return -scaleKg;
}

// Auto Mode Functionality
// void processAutoMode(float weight)
// {
//     if (waitingForExchange)
//     {
//         if ((millis() - exchangeStartTime) > AUTO_MODE_EXCHANGE_DELAY)
//         {
//             open();
//             waitingForExchange = false;
//             isValueOpen = true;
//             lcd.clear();
//         }
//     }
//     if (weight >= DISPENSER_WEIGHT)
//     {
//         close();
//         isValueOpen = false;
//         waitingForExchange = true;
//         exchangeStartTime = millis();
//     }
// }

// void open()
// {
//     Serial.println("Opening Dispenser");
// }

// void close()
// {
//     Serial.println("Closing Dispenser");
// }

void showInDisplay(String line1, String line2)
{
    if (lastDisplayText == line1 + line2)
    {
        return;
    }
    lcd.clear();
    lastDisplayText = line1 + line2;
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

String getModeName(Mode mode)
{
    switch (mode)
    {
    case AUTO:
        return "AUTO";
    case WEIGHTING:
        return "WEIGHTING";
    case MANUAL_SET:
        return "MANUAL_SET";
    case MANUAL_DISPENSE:
        return "MANUAL_DISPENSE";
    default:
        return "UNKNOWN";
    }
}