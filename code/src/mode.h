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
    ModeDefault(LoadCell &loadCell, UserInput &buttons, Display &display, Stopwatch &stopwatch)
        : loadCell(loadCell), buttons(buttons), display(display), stopwatch(stopwatch){};
    ~ModeDefault(){};
    void update();

private:
    LoadCell &loadCell;
    UserInput &buttons;
    Display &display;
    Stopwatch &stopwatch;
};

enum class CalibrationStep
{
    ENTER_WEIGHT,
    CALIBRATING,
    SUCCESS
};

class ModeCalibrateLoadCell : public Mode
{
public:
    ModeCalibrateLoadCell(LoadCell &loadCell, UserInput &buttons, Display &display, Stopwatch &stopwatch);
    ~ModeCalibrateLoadCell(){};
    void update();

private:
    LoadCell &loadCell;
    UserInput &buttons;
    Display &display;
    Stopwatch &stopwatch;

    static const unsigned int calibrationWeight = 100;
    CalibrationStep calibrationStep;
};
