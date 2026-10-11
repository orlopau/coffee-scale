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
    held = false;
    current = 0;
}

float AdaptiveAverage::add(uint32_t ms, float grams)
{
    // While averaging, one fast sample alone is likely noise: it is held back
    // until the next sample shows whether the weight really changed. That one
    // is compared with the average, the best guess of the weight so far,
    // rather than with a single sample that may be noisy itself, and over the
    // time of one sample, like the held one, so a small real step isn't taken
    // for slow. A held sample from back in time gives no time to measure over.
    const uint32_t sinceMs = held && heldMs > lastMs ? heldMs : lastMs;
    const float since = held ? current : buffer->getRelative(0);
    // Same time twice, or time running backwards, has no rate: like a fast change.
    bool slow = false;
    if (hasLast && ms > sinceMs)
    {
        float seconds = (ms - sinceMs) / 1000.0f;
        slow = std::fabs(grams - since) / seconds < threshold;
    }

    if (!slow && averaged > 0 && !held)
    {
        held = true;
        heldMs = ms;
        return current;
    }
    held = false;

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
