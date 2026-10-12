#pragma once

#include <stdint.h>

/**
 * Detects a lift: a finger sliding up the side of the platform lifts it a
 * little, so the weight dips below the steady weight before and comes right
 * back. Nothing else on the counter does that. Why lifting and where the
 * numbers come from: docs/adr/0001-tare-gesture.md.
 *
 * Reads the calibrated weight before filter, tare and zero tracking, so a
 * tare doesn't move what it watches.
 */
class LiftDetector
{
public:
    /** A steady stretch spans less than this. */
    static constexpr float STEADY_GRAMS = 0.3f;
    /** Readings that start a steady stretch. */
    static constexpr int STEADY_READINGS = 3;
    /** The weight before is the mean of at most this many readings, about 3 s. */
    static constexpr int MAX_STRETCH_READINGS = 32;
    /** A reading more than this below the weight before is part of a dip. */
    static constexpr float DIP_GRAMS = 0.3f;
    /** A reading within this of the weight before is back. */
    static constexpr float BACK_GRAMS = 0.3f;
    /** The most readings a lift's dip lasts. Lifts took 2–3, a cup taken off and put back 10. */
    static constexpr int MAX_DIP_READINGS = 5;
    /** Readings further apart reset the detector: a dip across a gap can't be judged. */
    static constexpr uint32_t MAX_GAP_MS = 500;

    /** Takes the next reading. True on a lift, with the weight from before it in weightBefore. */
    bool update(uint32_t ms, float grams, float &weightBefore);
    /** Grams a lift has to dip at least, 0 (the default) turns detection off. */
    void setDepth(float grams) { depth = grams; }
    /** Forgets the steady stretch and any dip, e.g. while a mode measures. */
    void reset();

private:
    /** Adds a reading outside a dip, to the last readings and the steady stretch. */
    void add(float grams);
    /** Forgets the steady stretch and starts over with this reading. */
    void startOver(float grams);
    void addToStretch(float grams);
    float stretchMean() const;

    float depth = 0;

    float recent[STEADY_READINGS] = {};
    int recentCount = 0;
    int recentNext = 0;

    float stretch[MAX_STRETCH_READINGS] = {};
    int stretchCount = 0;
    int stretchNext = 0;

    bool dipping = false;
    float before = 0;
    float deepest = 0;
    int dipLength = 0;

    bool hasLast = false;
    uint32_t lastMs = 0;
};
