#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "HX711.h"
#include <Valve.h>
#include <Automatic.h>
#include <PressButton.h>
#include <CustomScale.h>

// PARAMETERS
#define CALIBRATION_FACTOR -7050.0           // This value is obtained using the SparkFun_HX711_Calibration sketch
#define DISPENSER_WEIGHT 20                  // This is the weight of the rice that will be dispensed in kg for auto mode
#define INITIAL_LOADING_DELAY 5000           // This is for the initial loading of the rice at startup
#define AUTO_MODE_EXCHANGE_DELAY 20000       // This is the delay between the exchange of the container in auto mode
#define VALVE_OPEN_DELAY 1000                // this is the time for which the valve will be open and close
#define DISPENSER_WEIGHT_OFFSET 1            // this is the offset for the dispenser weight in kg to start valve closing
#define WEIGHT_MODE_TO_AUTO_MODE_DELAY 10000 // this is the delay to wait when switch back to auto mode from weight mode

// Sensors Pins
#define LOADCELL_DOUT_PIN_1 5
#define LOADCELL_SCK_PIN_1 4
#define LOADCELL_DOUT_PIN_2 3
#define LOADCELL_SCK_PIN_2 2

// Switches and Buttons Pins
#define AUTO_MODE_SWITCH 8         // if this is on, the system will automatically dispense the rice with preset weight, else the system will just act like a weighing machine
#define PLUS_SWITCH 10             // this will increase the weight in manual dispenser mode
#define MINUS_SWITCH 11            // this will decrease the weight in manual dispenser mode
#define MANUAL_DISPENSER_SWITCH 12 // this will turn on the manual dispenser mode
#define VALVE_PIN1 6               // this is the pin for the valve
#define VALVE_PIN2 7               // this is the pin for the valve
#define EMERGENCY_SWITCH 13

// initialize the liquid crystal library
// the first parameter is  the I2C address
// the second parameter is how many rows are on your screen
// the  third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27, 16, 2);                         // set the LCD address to 0x27 for a 16 chars and 2 line display
HX711 scale;                                                // Create a scale instance
Valve valve(VALVE_PIN1, VALVE_PIN2, VALVE_OPEN_DELAY);      // Create a valve instance
PressButton manualDispenserSwitch(MANUAL_DISPENSER_SWITCH); // Create a button instance
PressButton plusSwitch(PLUS_SWITCH);                        // Create a button instance
PressButton minusSwitch(MINUS_SWITCH);                      // Create a button instance
PressButton emergencySwitch(EMERGENCY_SWITCH);
CustomScale scale1(LOADCELL_DOUT_PIN_1, LOADCELL_SCK_PIN_1, CALIBRATION_FACTOR, false); // Create a scale instance
CustomScale scale2(LOADCELL_DOUT_PIN_2, LOADCELL_SCK_PIN_2, CALIBRATION_FACTOR, true);  // Create a scale instance
AlternateScaling alternateScaling(scale1, scale2);                                      // Create an alternate scaling instance
Automatic automatic(
    valve,
    DISPENSER_WEIGHT,
    DISPENSER_WEIGHT_OFFSET,
    AUTO_MODE_EXCHANGE_DELAY,
    alternateScaling,
    WEIGHT_MODE_TO_AUTO_MODE_DELAY
); // Create an automatic instance

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
int manualDispenserCustomWeight = DISPENSER_WEIGHT;
bool isPausePressed = false;

void showInDisplay(String, String);
String getModeName(Mode mode);

void setup()
{
    Serial.begin(9600);
    Serial.println("System is starting...");

    // Setup Pins
    pinMode(AUTO_MODE_SWITCH, INPUT_PULLUP);

    lcd.init();
    lcd.backlight();
    automatic.setLcdDisplayCallback(showInDisplay);

    Serial.println("Scale is ready");

    // Startup
    showInDisplay("Please Load Rice", "Upto " + String(DISPENSER_WEIGHT) + " kg");
    // delay for few seconds
    delay(INITIAL_LOADING_DELAY);
    valve.open();
    lcd.clear();
    Serial.println("System is ready");
}
void loop()
{
    valve.update();

    if (emergencySwitch.isPressed())
    {
        if (!isPausePressed)
        {
            isPausePressed = true;
            automatic.reset();
            currentMode = AUTO;
            manualDispenserCustomWeight = DISPENSER_WEIGHT;
        }
        else
        {
            isPausePressed = false;
            automatic.startSwitchModeDelay();
        }
    }
    if (isPausePressed)
    {
        showInDisplay("STOPPED", "Press to Restart");
        return;
    }

    int weight = alternateScaling.getPrimaryWeight();

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
    if (manualDispenserSwitch.isPressed())
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
        Serial.println("Switched to " + getModeName(currentMode));

        // If the current mode is auto, start the switch mode delay because we came back from other mode
        if (currentMode == AUTO)
        {
            automatic.startSwitchModeDelay();
        }
    }

    // Act based on the current mode
    if (currentMode == AUTO)
    {
        automatic.processAutoMode(); //  process the automatic mode. display will be updated by the callback
    }
    else if (currentMode == WEIGHTING)
    {
        showInDisplay("Mode: " + getModeName(currentMode), "Weight: " + String(weight));
    }
    else if (currentMode == MANUAL_SET)
    {
        valve.close();
        if (plusSwitch.isPressed())
        {
            manualDispenserCustomWeight += 1;
            Serial.println("Set Weight: " + String(manualDispenserCustomWeight));
        }
        else if (minusSwitch.isPressed())
        {
            manualDispenserCustomWeight -= 1;
            Serial.println("Set Weight: " + manualDispenserCustomWeight);
        }
        showInDisplay("Set Weight: " + String(manualDispenserCustomWeight), "Press + or -");
    }
    else if (currentMode == MANUAL_DISPENSE)
    {
        showInDisplay("Mode: " + getModeName(currentMode), "Weight: " + String(weight) + "/" + String(manualDispenserCustomWeight));
        valve.open();
        if (weight >= (manualDispenserCustomWeight - DISPENSER_WEIGHT_OFFSET))
        {
            valve.close();
            currentMode = MANUAL_SET;
            showInDisplay("Switched to " + getModeName(currentMode), "Press Dispense");
            Serial.println("Switched to " + getModeName(currentMode) + " Because weight is greater than set weight");
        }
    }
}

void showInDisplay(String line1, String line2)
{
    if (lastDisplayText == line1 + line2)
    {
        return;
    }
    Serial.println(line1 + " | " + line2);
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