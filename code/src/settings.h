#pragma once

#include <stdint.h>

struct Datapoint 
{
    const char *name;
}

enum class Setting : Datapoint
{
    AVERAGING_SAMPLES = 0x01,
    COFFEE_WEIGHT = 0x02,
    KASUYA_RATIO = 0x04,
    KASUYA_NUM_POURS = 0x08,
};

class Settings
{
public:
    long get(uint32_t setting);
    void set()
    virtual float getPercentage() = 0;
    virtual bool isCharging() = 0;
};