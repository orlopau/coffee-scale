#pragma once

#include <stdint.h>

/** One raw reading of the load cell, with the time it became ready. */
struct Sample
{
    uint32_t ms;
    int32_t raw;
};

/** Where the readings of the load cell come from, oldest first. */
class SampleSource
{
public:
    virtual ~SampleSource() {}
    /** Takes the next reading. False if there is none. */
    virtual bool next(Sample &sample) = 0;
};
