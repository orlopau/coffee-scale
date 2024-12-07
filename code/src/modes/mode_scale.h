#pragma once

#include "mode.h"
#include "scale.h"
#include "display.h"
#include "stopwatch.h"

class ModeScale : public Mode
{
public:
    ModeScale(WeightSensor &weightSensor, Display &display, Stopwatch &stopwatch)
        : weightSensor(weightSensor), display(display), stopwatch(stopwatch){};
    ~ModeScale(){};
    void update();
    bool canSwitchMode();
    const char* getName();

private:
    WeightSensor &weightSensor;
    Display &display;
    Stopwatch &stopwatch;
};