#pragma once

#include "weight_sensor.h"
#include "display.h"
#include "stopwatch.h"
#include "mode.h"

#define DEFAULT_CALIBRATION_WEIGHT 100
#define CALIBRATION_SAMPLE_SIZE 100

class ModeCalibration : public Mode
{
public:
    ModeCalibration(Stopwatch &stopwatch, void (*saveScaleFnc)(float));
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

    Stopwatch &stopwatch;
    void (*saveScaleFnc)(float);

    CalibrationStep calibrationStep;
    long sumMeasurements;
    unsigned int numMeasurements;
    long tare;
};
