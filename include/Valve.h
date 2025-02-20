#include <Arduino.h>

#ifndef VALVE_H
#define VALVE_H

class Valve
{
    int pin;

public:
    Valve(int pin)
    {
        this->pin = pin;
        pinMode(pin, OUTPUT);
    }

    void open()
    {
        digitalWrite(pin, HIGH);
    }
    void close()
    {
        digitalWrite(pin, LOW);
    }
};

#endif
