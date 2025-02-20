#include <Arduino.h>

#ifndef PressButton_H
#define PressButton_H

class PressButton
{
private:
    int pin;           // Pin connected to the button
    bool currentState; // Current button state
    bool lastState;    // Previous button state

public:
    // Constructor to initialize the button with its pin
    PressButton(int buttonPin) : pin(buttonPin), currentState(false), lastState(false)
    {
        pinMode(pin, INPUT_PULLUP); // Use internal pull-up resistor
    }

    // Method to check if the button was just pressed and released
    bool isPressed()
    {
        currentState = digitalRead(pin); // Read the button state

        // Detect state change from HIGH to LOW
        if (currentState == LOW && lastState == HIGH)
        {
            lastState = currentState; // Update the last state
            return true;              // Button was pressed and released
        }

        // Update the last state
        lastState = currentState;
        return false;
    }
};

// void setup()
// {
//     Serial.begin(9600);
// }

// void loop()
// {
//     // Check if the manual dispenser switch is pressed and released
//     if (manualDispenserSwitch.isPressedAndReleased())
//     {
//         Serial.println("Manual Dispenser Switch pressed and released!");
//     }
// }

#endif