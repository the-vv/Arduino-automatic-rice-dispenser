#include <Arduino.h>
#include <Valve.h>
#include <CustomScale.h>

#ifndef AUTOMATIC_H
#define AUTOMATIC_H

class Automatic
{
    Valve &valve;
    AlternateScaling &alternateScaling;
    bool waitingForExchange = false;
    unsigned long exchangeStartTime = 0;
    float dispenserWeight;
    float dispenserWeightOffset;
    unsigned long exchangeDelay;
    void (*showInDisplay)(String, String);

public:
    Automatic(Valve &valve, float dispenserWeight, float dispenserWeightOffset, unsigned long exchangeDelay, AlternateScaling &alternateScaling) : valve(valve), alternateScaling(alternateScaling)
    {
        this->dispenserWeight = dispenserWeight;
        this->dispenserWeightOffset = dispenserWeightOffset;
        this->exchangeDelay = exchangeDelay;
    }
    void reset()
    {
        waitingForExchange = false;
        exchangeStartTime = 0;
        valve.close();
    }
    void setLcdDisplayCallback(void (*showInDisplay)(String, String))
    {
        this->showInDisplay = showInDisplay;
    }
    void processAutoMode()
    {
        int weight = alternateScaling.getWeight();
        if (waitingForExchange)
        {
            if ((millis() - exchangeStartTime) < (exchangeDelay / 2))
            {
                showInDisplay("Secure your", "Dispenser");
            }
            else
            {
                showInDisplay("Waiting For", "Next Dispenser");
            }
            if ((millis() - exchangeStartTime) > exchangeDelay) // check if the exchange delay has passed, if so open the valve
            {
                valve.open();
                waitingForExchange = false;
                alternateScaling.switchScale(); // switch the scale after the exchange delay
            }
        }
        else
        {
            valve.open(); // make sure the valve is open
            showInDisplay("Mode: AUTO", "Weight: " + String(weight));
            if (weight >= (dispenserWeight - dispenserWeightOffset))
            {
                // close the valve and start the exchange delay if the weight is greater than the dispenser set weight
                valve.close();
                waitingForExchange = true;
                exchangeStartTime = millis();
            }
        }
    }
};

#endif