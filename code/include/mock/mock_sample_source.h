#pragma once

#include <deque>

#include "weighing/sample.h"

/** Hands out the samples a test queues, like the load cell's queue on the device. */
class QueuedSamples : public SampleSource
{
public:
    std::deque<Sample> samples;

    bool next(Sample &sample) override
    {
        if (samples.empty())
        {
            return false;
        }
        sample = samples.front();
        samples.pop_front();
        return true;
    }
};
