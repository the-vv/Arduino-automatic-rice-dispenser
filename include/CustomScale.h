#include <Arduino.h>
#include <HX711.h>

#ifndef CustomScale_H
#define CustomScale_H

class CustomScale
{
    HX711 scale;

public:
    CustomScale(int doutPin, int sckPin, float calibrationFactor)
    {
        scale.begin(doutPin, sckPin);
        scale.set_scale(calibrationFactor); // This value is obtained by using the SparkFun_HX711_Calibration sketch
        scale.tare();
    }

    int getWeight()
    {
        float scaleLbs = scale.get_units();
        float scaleKg = scaleLbs * 0.453592;
        return (int)-scaleKg;
        // return 2.0;
    }
};

class AlternateScaling
{
    CustomScale &scale1;
    CustomScale &scale2;

    byte activeScaleIndex = 0;

public:
    AlternateScaling(CustomScale &scale1, CustomScale &scale2) : scale1(scale1), scale2(scale2) {}

    float getPrimaryWeight()
    {
        return scale1.getWeight();
    }

    float getSecondaryWeight()
    {
        return scale2.getWeight();
    }

    float getWeight()
    {
        if (activeScaleIndex == 0)
        {
            return getPrimaryWeight();
        }
        else
        {
            return getSecondaryWeight();
        }
    }

    void switchScale()
    {
        activeScaleIndex = !activeScaleIndex;
    }
};

#endif
