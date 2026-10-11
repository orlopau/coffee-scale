#pragma once

#include <stdint.h>

#include "weighing/adaptive_average.h"
#include "weighing/flow_estimator.h"
#include "weighing/sample.h"

/** One sample of the load cell after all stages of the pipeline. */
struct Reading
{
    uint32_t ms;
    int32_t raw;
    /** Calibrated, not tared. */
    float grams;
    /** Tared, not averaged. */
    float net;
    /** Tared and averaged, what the display shows. */
    float filtered;
    /** Grams per second flowing onto the scale, not affected by taring. */
    float flow;
};

/**
 * Turns samples of the load cell into readings, stage by stage: calibration,
 * filter, tare and flow.
 */
class WeightPipeline
{
public:
    void setGramsPerCount(float gramsPerCount) { this->gramsPerCount = gramsPerCount; }
    /** See AdaptiveAverage::configure(). */
    void setAutoAveraging(float gramsPerSecond, uint8_t samples) { average.configure(gramsPerSecond, samples); }

    Reading process(const Sample &sample);
    /** Zeroes the averaged weight. Does nothing before the first sample. */
    void tare();

    bool hasReading() const { return started; }
    /** The reading of the last sample, all zero before the first. */
    const Reading &latest() const { return last; }

private:
    float gramsPerCount = 1;
    float offset = 0;
    AdaptiveAverage average;
    FlowEstimator flow;

    bool started = false;
    Reading last = {};
};
