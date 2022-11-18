#pragma once

class Battery
{
public:
    virtual ~Battery(){};
    virtual float getVoltage() = 0;
    virtual float getPercentage() = 0;
    virtual bool isCharging() = 0;
};