#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>

#include "weighing/weight_source.h"

enum class ShotEvent
{
    None,
    Accepted,
    PumpStopped
};

/**
 * Detects an espresso shot: after a tare the weight sits steady at about
 * 0 g, then rises smoothly, like no handful of beans and no bump does. A
 * rise is watched for 10 s before it is accepted; the shot then counts from
 * its first drop. The rules and their values are measured on recorded shots:
 * docs/adr/0002-shot-detection/README.md, built as in
 * docs/superpowers/specs/2026-10-20-shot-detection-design.md.
 *
 * Reads the calibrated weight before tare, filter and zero tracking
 * (Reading::grams), and net only to know whether the steady stretch is at
 * about 0 g. Every flow is the slope of a least-squares line through the
 * readings of a time window.
 */
class ShotDetector
{
public:
    /** A steady stretch spans this long. */
    static constexpr uint32_t STEADY_MS = 2000;
    /** A steady stretch flows less than this, either way. */
    static constexpr float STEADY_FLOW = 0.15f;
    /** A steady stretch's net is at most this far from 0: the scale was tared. */
    static constexpr float ARM_GRAMS = 0.3f;
    /** A steady stretch's readings scatter less than this around its line. */
    static constexpr float STEADY_MAX_SIGMA = 0.5f;
    /** The averaged weight is the mean of the readings this long up to a reading. */
    static constexpr uint32_t AVERAGE_MS = 300;
    /** A rise is noticed this many σ of the averaged weight above the baseline, */
    static constexpr float RISE_SIGMAS = 3;
    /** but at least this far. */
    static constexpr float RISE_MIN_GRAMS = 0.1f;
    /** The first drop follows the last reading averaged at most this many σ above the baseline. */
    static constexpr float DROP_SIGMAS = 1;
    /** The flow over FLOOR_WINDOW_MS begins the watch, and doesn't fall below it during the watch. */
    static constexpr float FLOOR_FLOW = 0.15f;
    static constexpr uint32_t FLOOR_WINDOW_MS = 2000;
    /** The floor applies from this long into the watch: at its start the flow is right at the floor. */
    static constexpr uint32_t FLOOR_FROM_MS = 1000;
    /** A rise that hasn't reached the floor this long after it was noticed was a step that stays. */
    static constexpr uint32_t FLOOR_WAIT_MS = 10000;
    /** How long a rise is watched before it is accepted. */
    static constexpr uint32_t WATCH_MS = 10000;
    /**
     * Smoothness: the flow over SMOOTH_SHORT_MS around a moment deviates from
     * the flow over SMOOTH_LONG_MS around it by at most SMOOTH_BASE_FLOW plus
     * SMOOTH_SIGMAS times the noise of the short flow.
     */
    static constexpr uint32_t SMOOTH_SHORT_MS = 250;
    static constexpr uint32_t SMOOTH_LONG_MS = 2000;
    static constexpr float SMOOTH_BASE_FLOW = 1.5f;
    static constexpr float SMOOTH_SIGMAS = 3;
    /** The flow over the whole watch is at most this: faster is a pour. */
    static constexpr float CEILING_FLOW = 5;
    /**
     * The pump stopped when the flow over the last STOP_WINDOW_MS falls below
     * STOP_FRACTION of the flow over the STOP_WINDOW_MS ending STOP_BEFORE_MS
     * earlier, and that earlier flow is at least FLOOR_FLOW. It stopped
     * STOP_LOOKBACK_MS before that reading: where the flow began to fall.
     */
    static constexpr uint32_t STOP_WINDOW_MS = 1000;
    static constexpr uint32_t STOP_BEFORE_MS = 2000;
    static constexpr float STOP_FRACTION = 0.5f;
    static constexpr uint32_t STOP_LOOKBACK_MS = 1000;
    /**
     * The cup is lifted while the weight is more than this below the shot's
     * highest. A reading that meets the pump stop is reported with the next
     * one, unless that one is lifted: the first reading of a lift can be only
     * partly down and look like a collapsing flow.
     */
    static constexpr float CUP_LIFTED_GRAMS = 5;
    /**
     * After a lift the pump stop is judged only once the weight has risen this
     * far above where it was when the cup came back. If the pump stopped while
     * the cup was off, only drips follow (at most 1.2 g), and the noise of a
     * flat cup must not end the shot: a click does.
     */
    static constexpr float CUP_BACK_RISE_GRAMS = 2;
    /** Readings further apart reset the detector. */
    static constexpr uint32_t MAX_GAP_MS = 500;
    /** Readings kept, about 27 s: the steady stretch, the rise, the watch and the windows around them. */
    static constexpr int HISTORY_READINGS = 300;

    ShotDetector() { risen.reserve(HISTORY_READINGS); }

    /** Takes the next reading. Accepted once per shot, then PumpStopped once. */
    ShotEvent update(const Reading &reading);
    /** The accepted shot's first drop, the first reading of rise(). */
    uint32_t firstDropMs() const { return risen.empty() ? 0 : risen.front().ms; }
    /** The accepted shot's readings, from the first drop on. */
    const std::vector<Reading> &rise() const { return risen; }
    /** When the pump stopped, valid after PumpStopped. */
    uint32_t pumpStopMs() const { return pumpStop; }
    /** True from Accepted until PumpStopped or reset(). */
    bool isShotRunning() const { return state == State::Running; }
    /** Forgets everything, it needs a steady stretch at 0 g again. */
    void reset();

private:
    enum class State
    {
        /** For a steady stretch at 0 g. */
        Waiting,
        /** Was steady at 0 g, for a rise; follows the stretch while it stays steady. */
        Armed,
        /** For the flow to reach the floor. */
        Rising,
        /** Judges the rise. */
        Watching,
        /** An accepted shot, for the pump stop. After it, waiting again: for the next tare. */
        Running
    };

    struct Point
    {
        uint32_t ms;
        float grams;
        /** The mean grams over AVERAGE_MS up to here. */
        float average;
    };

    /** A least-squares line, times in seconds. */
    struct Line
    {
        float slope;
        float mean;
        /** The scatter around the line. */
        float sigma;
        /** Σ(t − t̄)², how well the times pin the slope. */
        float sumSquaredTimeDeviations;
    };

    /** Adds a reading to the history, with its averaged weight. */
    void remember(const Reading &reading);
    /** The point k readings before the newest. */
    const Point &back(int k) const { return history[(next - 1 - k + HISTORY_READINGS) % HISTORY_READINGS]; }
    /** The line through the grams, or the averaged weights, of the readings with ms in (fromMs, toMs]. */
    Line fit(uint32_t fromMs, uint32_t toMs, bool averaged = false) const;

    /** Waiting and armed: arms on a steady stretch at 0 g, follows it and notices a rise. */
    void rest(const Reading &reading);
    ShotEvent rising(const Reading &reading);
    ShotEvent watching(const Reading &reading);
    ShotEvent running(const Reading &reading);
    /** Drops the readings before the first drop from risen. */
    void trimToFirstDrop();
    /** False if the moment at ms breaks the smoothness. */
    bool isSmooth(uint32_t ms) const;
    /** Back to waiting. */
    void reject();

    State state = State::Waiting;

    Point history[HISTORY_READINGS] = {};
    int count = 0;
    int next = 0;

    /** The last steady stretch's mean grams, its readings' σ around its line, and that of their averaged weight. */
    float baseline = 0;
    float sigma = 0;
    float averageSigma = 0;

    uint32_t noticedMs = 0;
    uint32_t watchStartMs = 0;
    /** The last moment of the watch judged for smoothness. */
    uint32_t judgedMs = 0;
    /** From the first drop on; until accepted, from the reading after the last one at rest. */
    std::vector<Reading> risen;
    /** The shot's highest grams since the first drop. */
    float highest = 0;
    /** The pump stop is judged from here on: the stop windows don't reach into a lift. */
    uint32_t judgeStopFromMs = 0;
    /** The last reading was lifted. */
    bool lifted = false;
    /** After a lift, until the weight has risen CUP_BACK_RISE_GRAMS above cupBackGrams. */
    bool waitForRise = false;
    /** The weight when the cup came back. */
    float cupBackGrams = 0;
    /** The last reading met the pump stop; reported with the next one if that isn't lifted. */
    bool stopCandidate = false;
    uint32_t pumpStopCandidate = 0;
    uint32_t pumpStop = 0;

    bool hasLast = false;
    uint32_t lastMs = 0;
};
