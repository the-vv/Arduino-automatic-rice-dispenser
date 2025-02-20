#include <Arduino.h>
#include <Valve.h>

#ifndef AUTOMATIC_H
#define AUTOMATIC_H

class Automatic
{
    Valve valve;
    bool waitingForExchange = false;
    unsigned long exchangeStartTime = 0;
    float dispenserWeight;
    unsigned long exchangeDelay;

public:
    Automatic(int valvePin, float dispenserWeight, unsigned long exchangeDelay) : valve(valvePin)
    {
        this->dispenserWeight = dispenserWeight;
        this->exchangeDelay = exchangeDelay;
    }
    void reset()
    {
        waitingForExchange = false;
        exchangeStartTime = 0;
        valve.close();
    }
    void processAutoMode(float weight)
    {
        if (waitingForExchange)
        {
            if ((millis() - exchangeStartTime) > exchangeDelay) // check if the exchange delay has passed, if so open the valve
            {
                valve.open();
                waitingForExchange = false;
            }
        }
        if (weight >= dispenserWeight)
        {
            // close the valve and start the exchange delay if the weight is greater than the dispenser set weight
            valve.close();
            waitingForExchange = true;
            exchangeStartTime = millis();
        }
    }
};

#endif