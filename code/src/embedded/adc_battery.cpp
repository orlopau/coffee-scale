#ifndef NATIVE

#include "Arduino.h"
#include "battery.h"
#include "constants.h"

namespace Battery
{
    static constexpr float multiplier = 2 * 0.996116504854369;
    static constexpr float offset = 0;
    static constexpr float min = 3;
    static constexpr float max = 4.2;

    static bool isInit = false;

    void init()
    {
        pinMode(PIN_BAT_CHARGE_STATUS, INPUT_PULLUP);
        pinMode(PIN_BAT_ADC, INPUT);
        isInit = true;
    }

    float getVoltage()
    {
        if (!isInit)
        {
            return -1.0f;
        }
        double mV = analogReadMilliVolts(PIN_BAT_ADC);
        // reading = -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) +
        // 0.001109019271794 * reading + 0.034143524634089;
        return (mV * multiplier) / 1000 + offset;
    }

    float getPercentage()
    {
        if (!isInit)
        {
            return -1.0f;
        }

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

    bool isCharging()
    {
        if (!isInit)
        {
            return false;
        }

        return digitalRead(PIN_BAT_CHARGE_STATUS) == LOW;
    }
}

#endif