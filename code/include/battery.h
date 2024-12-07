#pragma once

namespace Battery
{
    void init();
    float getVoltage();
    float getPercentage();
    bool isCharging();
};