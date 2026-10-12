#include <math.h>

#include "detectors/lift_detector.h"

/** Highest minus lowest of the values and one more. */
static float spanWith(const float *values, int count, float more)
{
    float lowest = more;
    float highest = more;
    for (int i = 0; i < count; i++)
    {
        lowest = fminf(lowest, values[i]);
        highest = fmaxf(highest, values[i]);
    }
    return highest - lowest;
}

void LiftDetector::reset()
{
    recentCount = 0;
    recentNext = 0;
    stretchCount = 0;
    stretchNext = 0;
    dipping = false;
    hasLast = false;
}

bool LiftDetector::update(uint32_t ms, float grams, float &weightBefore)
{
    if (hasLast && ms - lastMs > MAX_GAP_MS)
    {
        reset();
    }
    hasLast = true;
    lastMs = ms;

    if (dipping)
    {
        if (fabsf(grams - before) <= BACK_GRAMS)
        {
            dipping = false;
            const bool lift = depth > 0 && deepest >= depth;
            startOver(grams);
            if (lift)
            {
                weightBefore = before;
            }
            return lift;
        }
        if (grams < before - DIP_GRAMS && dipLength < MAX_DIP_READINGS)
        {
            dipLength++;
            deepest = fmaxf(deepest, before - grams);
            return false;
        }
        // too long for a lift, e.g. a cup taken off, or something was put on
        dipping = false;
        startOver(grams);
        return false;
    }

    if (stretchCount > 0 && grams < stretchMean() - DIP_GRAMS)
    {
        dipping = true;
        before = stretchMean();
        deepest = before - grams;
        dipLength = 1;
        return false;
    }
    add(grams);
    return false;
}

void LiftDetector::add(float grams)
{
    recent[recentNext] = grams;
    recentNext = (recentNext + 1) % STEADY_READINGS;
    if (recentCount < STEADY_READINGS)
    {
        recentCount++;
    }

    // bounding the whole stretch, not just the last readings, keeps a slow drift from pulling the mean behind
    if (stretchCount > 0 && spanWith(stretch, stretchCount, grams) < STEADY_GRAMS)
    {
        addToStretch(grams);
        return;
    }

    // the stretch ended, or there was none: a new one starts with the last readings, if they are steady
    stretchCount = 0;
    stretchNext = 0;
    if (recentCount == STEADY_READINGS && spanWith(recent, recentCount, recent[0]) < STEADY_GRAMS)
    {
        for (int i = 0; i < recentCount; i++)
        {
            addToStretch(recent[i]);
        }
    }
}

void LiftDetector::startOver(float grams)
{
    recentCount = 0;
    recentNext = 0;
    stretchCount = 0;
    stretchNext = 0;
    add(grams);
}

void LiftDetector::addToStretch(float grams)
{
    stretch[stretchNext] = grams;
    stretchNext = (stretchNext + 1) % MAX_STRETCH_READINGS;
    if (stretchCount < MAX_STRETCH_READINGS)
    {
        stretchCount++;
    }
}

float LiftDetector::stretchMean() const
{
    float sum = 0;
    for (int i = 0; i < stretchCount; i++)
    {
        sum += stretch[i];
    }
    return sum / stretchCount;
}
