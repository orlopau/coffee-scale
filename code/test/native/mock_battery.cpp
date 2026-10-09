#include "mock/mock_battery.h"

namespace Battery
{
    float voltage = 4.2;
    float percentage = 100;
    bool charging = false;

    void reset()
    {
        voltage = 4.2;
        percentage = 100;
        charging = false;
    }

    float getVoltage() { return voltage; }
    float getPercentage() { return percentage; }
    bool isCharging() { return charging; }
};
