#include "battery.h"
#include "constants.h"
#include "Arduino.h"

float batteryVoltage()
{
    double reading = analogRead(PIN_V_BAT);
    if (reading < 1 || reading > 4095)
        return 0;
    reading = -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
    return (reading + ADC_OFFSET) * BAT_V_MULTIPLIER;
}

float batteryPercentage()
{
    float voltage = batteryVoltage();
    if (voltage > BAT_V_MAX)
    {
        return 100;
    }
    else if (voltage < BAT_V_MIN)
    {
        return 0;
    }
    else
    {
        return (voltage - BAT_V_MIN / (BAT_V_MAX - BAT_V_MIN)) * 100;
    }
}

bool batteryCharging()
{
    return digitalRead(PIN_BAT_CHARGE_STATUS) == LOW;
}