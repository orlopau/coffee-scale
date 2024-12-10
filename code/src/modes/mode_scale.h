#pragma once

#include "mode.h"
#include "scale.h"
#include "stopwatch.h"

class ModeScale : public Mode
{
public:
    ModeScale(WeightSensor &weightSensor, Stopwatch &stopwatch)
        : weightSensor(weightSensor), stopwatch(stopwatch){};
    ~ModeScale(){};
    void update();
    bool canSwitchMode();
    const char* getName();

private:
    WeightSensor &weightSensor;
    Stopwatch &stopwatch;
};