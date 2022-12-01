#pragma once

#include "loadcell.h"
#include "user_interface.h"
#include "stopwatch.h"

class Mode
{
public:
    virtual ~Mode() {}
    virtual void update() = 0;
    virtual bool canSwitchMode() = 0;
    virtual const char* getName() = 0;
};

class ModeDefault : public Mode
{
public:
    ModeDefault(LoadCell &loadCell, UserInput &buttons, Display &display, Stopwatch &stopwatch)
        : loadCell(loadCell), buttons(buttons), display(display), stopwatch(stopwatch){};
    ~ModeDefault(){};
    void update();
    bool canSwitchMode();
    const char* getName();

private:
    LoadCell &loadCell;
    UserInput &buttons;
    Display &display;
    Stopwatch &stopwatch;
};