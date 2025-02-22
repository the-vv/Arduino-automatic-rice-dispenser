#include <Arduino.h>

#ifndef VALVE_H
#define VALVE_H

class Valve
{
    uint8_t pin1;
    uint8_t pin2;
    bool state;
    uint8_t openDelay;

public:
    Valve(uint8_t pin1, uint8_t pin2, uint8_t openDelay) : pin1(pin1), pin2(pin2), state(false), openDelay(openDelay)
    {
        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
    }

    void open()
    {
        if (this->state)
        {
            return;
        }
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
        this->state = true;
        delay(openDelay);
        stop();
    }
    void close()
    {
        if (!this->state)
        {
            return;
        }
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
        this->state = false;
        delay(openDelay);
        stop();
    }
    void stop()
    {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
    }
};

#endif
