#pragma once

#include "loadcell.h"
#include "user_interface.h"
#include "stopwatch.h"

class Mode
{
public:
    virtual ~Mode() {}
    virtual void update() = 0;
};

class ModeDefault : public Mode
{
public:
    ModeDefault(LoadCell &loadCell, Buttons &buttons, Display &display, Stopwatch &stopwatch)
        : loadCell(loadCell), buttons(buttons), display(display), stopwatch(stopwatch){};
    ~ModeDefault() {};
    void update();

private:
    LoadCell &loadCell;
    Buttons &buttons;
    Display &display;
    Stopwatch &stopwatch;
};
