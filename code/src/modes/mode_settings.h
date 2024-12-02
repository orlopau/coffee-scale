#pragma once

#include "mode.h"
#include "loadcell.h"
#include "user_interface.h"
#include "util/stopwatch.h"

class ModeSettings : public Mode
{
public:
    ModeSettings(WeightSensor &weightSensor, UserInput &buttons, Display &display, Stopwatch &stopwatch)
        : buttons(buttons), display(display){};
    ~ModeSettings(){};
    void update();
    bool canSwitchMode();
    const char* getName();

private:
    UserInput &buttons;
    Display &display;
};