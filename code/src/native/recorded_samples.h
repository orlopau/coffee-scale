#pragma once

#include "millis.h"
#include "native/recorded_scale.h"
#include "weighing/sample.h"

/** A recording's samples, timestamped when they become ready, like the device's load cell task. */
class RecordedSamples : public SampleSource
{
public:
    explicit RecordedSamples(RecordedScale &scale) : scale(scale) {}

    bool next(Sample &sample) override
    {
        if (!scale.isReady())
        {
            return false;
        }
        sample = {static_cast<uint32_t>(now()), static_cast<int32_t>(scale.read())};
        if (scale.elapsed() >= addedFromMs)
        {
            sample.raw += addedCounts;
        }
        return true;
    }

    /** Adds counts to every sample from a time in the recording on, e.g. a bean placed on the scale. */
    void add(int32_t counts, unsigned long fromMs)
    {
        addedCounts = counts;
        addedFromMs = fromMs;
    }

private:
    RecordedScale &scale;
    int32_t addedCounts = 0;
    unsigned long addedFromMs = ~0UL;
};
