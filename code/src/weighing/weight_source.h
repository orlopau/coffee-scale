#pragma once

#include <stdint.h>
#include <vector>

/** One sample of the load cell after all stages of the weight pipeline. */
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

/** The weight for the modes: the readings of this loop pass, the weight to show, and taring. */
class WeightSource
{
public:
    virtual ~WeightSource() {}
    /** All readings since the last loop pass, oldest first. Empty if there were none. */
    virtual const std::vector<Reading> &newReadings() = 0;
    /** The tared weight in grams to show, averaged while it holds still. */
    virtual float getWeight() = 0;
    virtual void tare() = 0;
};
