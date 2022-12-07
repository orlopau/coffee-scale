#pragma once

#include "loadcell.h"
#include "user_interface.h"
#include "stopwatch.h"
#include "mode.h"

#define DEFAULT_CALIBRATION_WEIGHT 100
#define CALIBRATION_SAMPLE_SIZE 100

class ModeCalibration : public Mode
{
public:
    ModeCalibration(WeightSensor &weightSensor, UserInput &buttons, Display &display, Stopwatch &stopwatch, void (*saveScaleFnc)(float));
    ~ModeCalibration(){};
    void update();
    const char* getName();
    bool canSwitchMode();

private:
    enum class CalibrationStep
    {
        BEGIN,
        ADD_WEIGHT,
        CALIBRATING,
        END
    };

    WeightSensor &weightSensor;
    UserInput &buttons;
    Display &display;
    Stopwatch &stopwatch;
    void (*saveScaleFnc)(float);

    CalibrationStep calibrationStep;
    unsigned long sumMeasurements;
    unsigned int numMeasurements;
};
