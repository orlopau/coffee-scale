#pragma once

#include "mode.h"
#include "weight_sensor.h"
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