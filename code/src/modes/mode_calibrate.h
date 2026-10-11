#pragma once

#include "weighing/weight_source.h"
#include "stopwatch.h"
#include "mode.h"

#define DEFAULT_CALIBRATION_WEIGHT 100
#define CALIBRATION_SAMPLE_SIZE 100

class ModeCalibration : public Mode
{
public:
    ModeCalibration(WeightSource &weightSource, Stopwatch &stopwatch, void (*saveScaleFnc)(float));
    ~ModeCalibration(){};
    void update() override;
    void render(Canvas &canvas) override;
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

    WeightSource &weightSource;
    Stopwatch &stopwatch;
    void (*saveScaleFnc)(float);

    CalibrationStep calibrationStep;
    long sumMeasurements;
    unsigned int numMeasurements;
    long tare;
    float scale;
};
