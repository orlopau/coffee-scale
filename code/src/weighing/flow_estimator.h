#pragma once

#include <stdint.h>

#include "ring_buffer.h"

/**
 * The flow stage: how many grams per second flow onto the scale, from a
 * straight line fitted (least squares) through the samples of the last
 * window.
 *
 * Times are taken relative to the newest sample, so the result stays precise
 * however long the scale has been running.
 */
class FlowEstimator
{
public:
    static constexpr uint32_t DEFAULT_WINDOW_MS = 2000;
    /** More than enough for the window at the HX711's 10 samples per second. */
    static constexpr uint8_t CAPACITY = 64;

    explicit FlowEstimator(uint32_t windowMs = DEFAULT_WINDOW_MS);

    /** Adds a sample and returns the flow in g/s, 0 with fewer than two samples in the window. */
    float add(uint32_t ms, float grams);
    void reset();

private:
    struct Point
    {
        uint32_t ms;
        float grams;
    };

    uint32_t windowMs;
    RingBuffer<Point> points;
};
