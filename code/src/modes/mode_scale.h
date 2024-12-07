#pragma once

#include "mode.h"
#include "scale.h"
#include "user_interface.h"
#include "stopwatch.h"

class ModeScale : public Mode
{
public:
    ModeScale(WeightSensor &weightSensor, UserInput &buttons, Display &display, Stopwatch &stopwatch)
        : weightSensor(weightSensor), buttons(buttons), display(display), stopwatch(stopwatch){};
    ~ModeScale(){};
    void update();
    bool canSwitchMode();
    const char* getName();

private:
    WeightSensor &weightSensor;
    UserInput &buttons;
    Display &display;
    Stopwatch &stopwatch;
};