#include "weighing/adaptive_average.h"

#include <cmath>

AdaptiveAverage::AdaptiveAverage() : buffer(new RingBuffer<float>(1)) {}

void AdaptiveAverage::configure(float gramsPerSecond, uint8_t samples)
{
    threshold = gramsPerSecond;
    buffer.reset(new RingBuffer<float>(samples > 0 ? samples : 1));
    reset();
}

void AdaptiveAverage::reset()
{
    buffer->clear();
    averaged = 0;
    hasLast = false;
    current = 0;
}

float AdaptiveAverage::add(uint32_t ms, float grams)
{
    // Same time twice, or time running backwards, has no rate: like a fast change.
    bool slow = false;
    if (hasLast && ms > lastMs)
    {
        float seconds = (ms - lastMs) / 1000.0f;
        slow = std::fabs(grams - buffer->getRelative(0)) / seconds < threshold;
    }

    buffer->push(grams);
    hasLast = true;
    lastMs = ms;

    if (slow)
    {
        if (averaged < buffer->capacity())
        {
            averaged++;
        }
    }
    else
    {
        averaged = 0;
    }

    current = averaged > 0 ? buffer->averageLast(averaged) : grams;
    return current;
}
