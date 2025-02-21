#include <Arduino.h>

#ifndef VALVE_H
#define VALVE_H

class Valve
{
    int pin;
    bool state;

public:
    Valve(int pin)
    {
        this->pin = pin;
        pinMode(pin, OUTPUT);
    }

    void open()
    {
        if (this->state)
        {
            return;
        }
        digitalWrite(pin, HIGH);
        this->state = true;
    }
    void close()
    {
        if (!this->state)
        {
            return;
        }
        digitalWrite(pin, LOW);
        this->state = false;
    }
};

#endif
