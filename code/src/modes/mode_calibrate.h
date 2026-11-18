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
    /** Keeps the latest raw value and counts the readings per second. */
    void noteReading(const Reading &reading);

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
    /** The latest raw reading, shown on every step to see the load cell work. */
    long lastRaw;
    /**
     * Readings in the last full second. 10 or 80 from a working HX711, 5 if
     * its data pin stays low and the load cell task reads on its timeout.
     */
    unsigned int rate;
    unsigned int rateCount;
    uint32_t rateStartMs;
    /** False until the first reading starts the first second. */
    bool rateStarted;
    float average;
    float scale;
};
