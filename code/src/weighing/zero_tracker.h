#pragma once

#include <stdint.h>

/**
 * The zero tracking stage: pulls a weight near zero slowly back to zero, so
 * the scale keeps reading 0.00 g while it drifts, e.g. as it warms up.
 *
 * Like the zero tracking of trade scales, it only moves the zero while the
 * weight sits still within a small window around zero, and only at a limited
 * speed, so anything bigger than the window, or anything flowing, is left
 * alone. While a mode measures, it holds the zero where it is.
 */
class ZeroTracker
{
public:
    /** The fastest the zero moves. A cold start drifted at most 0.003 g/s. */
    static constexpr float GRAMS_PER_SECOND = 0.01f;
    /** A flow above this isn't steady. At rest the flow stayed under 0.067 g/s. */
    static constexpr float STEADY_GRAMS_PER_SECOND = 0.1f;
    /** The most time one step makes up for, so a gap gives no big step. */
    static constexpr uint32_t MAX_STEP_MS = 200;

    /** Grams around zero that are tracked, 0 turns tracking off. */
    void setWindow(float grams) { window = grams; }
    /** While held, the zero stays where it is, e.g. during a shot. */
    void hold(bool held) { this->held = held; }
    /** Forgets the last reading, e.g. after taring. */
    void reset() { hasLast = false; }

    /**
     * Takes the tared weight and the flow of a reading, and returns the grams
     * to move the zero by: the weight minus them is the new tared weight.
     */
    float track(uint32_t ms, float grams, float flow);

private:
    float window = 0;
    bool held = false;
    bool hasLast = false;
    uint32_t lastMs = 0;
};
