#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "weighing/weight_source.h"

/**
 * Detects the first drops of an espresso: after a tare the weight sits
 * steady at about 0 g, then rises slowly and steadily. Bumps come back,
 * pouring flows too fast, a step stops flowing. A rise is judged until it
 * flows like a shot, so a slow start isn't lost. See
 * docs/superpowers/specs/2026-10-20-shot-detection-design.md.
 *
 * Reads the calibrated weight before tare, filter and zero tracking
 * (Reading::grams), and net only to know whether the scale was tared.
 */
class ShotDetector
{
public:
    // placeholders, tuned from the shot recordings
    /** Steady readings span less than this. */
    static constexpr float STEADY_GRAMS = 0.3f;
    /** Steady readings flow less than this, so a slow trickle never counts as steady. */
    static constexpr float STEADY_FLOW = 0.1f;
    /** Readings that make a steady stretch. */
    static constexpr int STEADY_READINGS = 3;
    /** The steady stretch's net is at most this far from 0: the scale was tared. */
    static constexpr float ARM_GRAMS = 0.5f;
    /** A reading this far above the baseline begins a rise. */
    static constexpr float RISE_GRAMS = 0.3f;
    /**
     * The first drop is the first reading this far above the baseline, before
     * the rise began: the first drops weigh less than RISE_GRAMS.
     */
    static constexpr float DROP_GRAMS = 0.1f;
    /**
     * A rise that falls back to this above the baseline was a bump. Lower
     * than RISE_GRAMS, so noise on the first drops doesn't end the rise.
     */
    static constexpr float BACK_GRAMS = 0.15f;
    /** A rise is judged from this long after its first drop on. */
    static constexpr uint32_t CONFIRM_MS = 2000;
    /** A rise that hasn't flowed like a shot this long after its first drop is given up. */
    static constexpr uint32_t MAX_RISE_MS = 10000;
    /** The accepted flow is between these times the expected flow. */
    static constexpr float MIN_FLOW_FACTOR = 0.3f;
    static constexpr float MAX_FLOW_FACTOR = 3.0f;
    /** Readings further apart reset the detector. */
    static constexpr uint32_t MAX_GAP_MS = 500;
    /** Readings before a rise that the first drop is looked for in, 2 s at 10 per second. */
    static constexpr int LOOKBACK_READINGS = 20;
    /** Readings a rise keeps, more than LOOKBACK_READINGS and MAX_RISE_MS at 10 per second. */
    static constexpr size_t MAX_RISE_READINGS = 128;

    ShotDetector() { risen.reserve(MAX_RISE_READINGS); }

    /** Grams per second a shot flows on average, e.g. 36 g in 30 s; sets the accepted range. */
    void setExpectedFlow(float gramsPerSecond) { expectedFlow = gramsPerSecond; }
    /** Takes the next reading. True once, when a rise is confirmed as a shot. */
    bool update(const Reading &reading);
    /** Time of the confirmed shot's first drop, the first reading of rise(). */
    uint32_t firstDropMs() const { return risen.empty() ? 0 : risen.front().ms; }
    /** The confirmed shot's readings, from the first drop on. */
    const std::vector<Reading> &rise() const { return risen; }
    /** Forgets everything, it needs a new steady stretch at 0 g to be armed again. */
    void reset();

private:
    /** Adds a reading outside a rise; arms on a steady stretch at about 0 g net. */
    void addSteady(const Reading &reading);
    /** Begins a rise at reading, from the first drop before it on. */
    void beginRise(const Reading &reading);
    /** Takes a reading of a rise. True if it confirms the shot. */
    bool addRise(const Reading &reading);
    /** Waits for a new steady stretch. */
    void disarm();

    float expectedFlow = 1.2f;

    Reading recent[STEADY_READINGS] = {};
    int recentCount = 0;
    int recentNext = 0;

    /** The last readings outside a rise. */
    Reading before[LOOKBACK_READINGS] = {};
    int beforeCount = 0;
    int beforeNext = 0;

    bool armed = false;
    float baseline = 0;
    bool rising = false;
    std::vector<Reading> risen;

    bool hasLast = false;
    uint32_t lastMs = 0;
};
