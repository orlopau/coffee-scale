#ifndef NATIVE

#include "battery.h"

class ADCBattery : public Battery
{
public:
    ADCBattery(unsigned int pinAdc, unsigned int pinState, float multiplier, float offset, float min, float max);
    float getVoltage();
    float getPercentage();
    bool isCharging();

private:
    unsigned int pinAdc;
    unsigned int pinState;
    float multiplier;
    float offset;
    float min;
    float max;
};

#endif