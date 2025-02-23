#include <Arduino.h>

#ifndef VALVE_H
#define VALVE_H

class Valve
{
    uint8_t pin1;
    uint8_t pin2;
    bool state;
    unsigned long openDelay;
    unsigned long lastTriggeredTime = 0;

public:
    Valve(uint8_t pin1, uint8_t pin2, unsigned long openDelay) : pin1(pin1), pin2(pin2), state(false), openDelay(openDelay)
    {
        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
    }
    void update() {
        if (lastTriggeredTime > 0 && (millis() - lastTriggeredTime) > openDelay) {
            stop();
            lastTriggeredTime = 0;
        }
    }

    void open()
    {
        if (this->state)
        {
            return;
        }
        Serial.println("Opening Valve");
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
        this->state = true;
        lastTriggeredTime = millis();
    }
    void close()
    {
        if (!this->state)
        {
            return;
        }
        Serial.println("Closing Valve");
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
        this->state = false;
        lastTriggeredTime = millis();
    }
    void stop()
    {
        Serial.println("Stopping Valve");
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
    }
};

#endif
