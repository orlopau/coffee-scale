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
    BEGIN,
    ENTER_WEIGHT,
    CALIBRATING,
    SUCCESS,
    END
};

class ModeCalibrateLoadCell : public Mode
{
public:
    ModeCalibrateLoadCell(LoadCell &loadCell, UserInput &buttons, Display &display, Stopwatch &stopwatch, void (*saveScaleFnc)(float));
    ~ModeCalibrateLoadCell(){};
    void update();

private:
    LoadCell &loadCell;
    UserInput &buttons;
    Display &display;
    Stopwatch &stopwatch;
    void (*saveScaleFnc)(float);

    static const unsigned int calibrationWeight = 100;
    CalibrationStep calibrationStep;
};
