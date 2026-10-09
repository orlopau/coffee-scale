#pragma once

#include "battery.h"

namespace Battery
{
    extern float voltage;
    extern float percentage;
    extern bool charging;

    void reset();
}
