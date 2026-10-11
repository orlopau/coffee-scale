#pragma once

#include <stdint.h>
#include <memory>

#include "ring_buffer.h"

/**
 * The filter stage: averages the weight while it changes slowly, so it holds
 * still on the display, and follows it directly while it changes fast.
 *
 * While the weight changes by less than the threshold per second, every new
 * sample adds one to the number of samples averaged, up to the maximum. A
 * faster change starts over with the last sample alone. Only samples since
 * averaging started are averaged, so the weight doesn't jump back towards
 * values from before.
 */
class AdaptiveAverage
{
public:
    AdaptiveAverage();

    /**
     * @param gramsPerSecond change per second under which samples are
     * averaged, 0 to never average
     * @param samples the most samples averaged
     */
    void configure(float gramsPerSecond, uint8_t samples);
    /** Forgets all samples, keeps the configuration. */
    void reset();
    /** Adds a sample and returns the averaged weight. */
    float add(uint32_t ms, float grams);
    /** True before the first sample, and after reset(). */
    bool isEmpty() const { return !hasLast; }
    /** The averaged weight after the last sample, 0 before the first. */
    float value() const { return current; }

private:
    std::unique_ptr<RingBuffer<float>> buffer;
    float threshold = 0;
    unsigned int averaged = 0;
    bool hasLast = false;
    uint32_t lastMs = 0;
    float current = 0;
};
