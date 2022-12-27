#ifndef NATIVE

#include "adc_battery.h"
#include "Arduino.h"

ADCBattery::ADCBattery(unsigned int pinAdc, unsigned int pinState, float multiplier, float offset, float min, float max)
    : pinAdc(pinAdc), pinState(pinState), multiplier(multiplier), offset(offset), min(min), max(max)
{
    pinMode(pinState, INPUT_PULLUP);
    pinMode(pinAdc, INPUT);
}

float ADCBattery::getVoltage()
{
    double mV = analogReadMilliVolts(pinAdc);
    // reading = -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
    return (mV * multiplier) / 1000 + offset;
}

float ADCBattery::getPercentage()
{
    float voltage = getVoltage();
    if (voltage > max)
    {
        return 100;
    }
    else if (voltage < min)
    {
        return 0;
    }
    else
    {
        return ((voltage - min) / (max - min)) * 100;
    }
}

bool ADCBattery::isCharging()
{
    return digitalRead(pinState) == LOW;
}

#endif