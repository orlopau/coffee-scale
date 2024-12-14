#include <math.h>

#include "auto_tare.h"
#include "logger.h"

#define TAG "Auto Tare"

#define STABLE_WEIGHT_DIFF 2.0f

bool isCloseTo(float toleranceAbs, float a, float b) { return abs(a - b) <= toleranceAbs; }

AutoTare::AutoTare(float toleranceG, float maxStdDevG, uint16_t bufferSize)
    : tolerance(toleranceG), maxStdDev(maxStdDevG), buffer(bufferSize)
{
}

void AutoTare::update(float rawWeight)
{
    buffer.push(rawWeight);

    // do nothing if bufer isnt full yet
    if (buffer.capacity() != buffer.size())
    {
        return;
    }

    // check if stable, i.e. stdDev is under threshold
    if (buffer.standardDeviationLast(buffer.size()) < maxStdDev)
    {
        float avgWeight = buffer.averageLast(buffer.size());

        // only use stable weight if there is some siginificant diff
        if (!isCloseTo(STABLE_WEIGHT_DIFF, lastStableWeight, avgWeight))
        {
            float diff = avgWeight - lastStableWeight;

            // only tare if the diff is positive
            if (diff > 0)
            {
                LOGI(TAG, "got new diff: %f\n", diff);

                // if the diff is similar to any saved weight, tare
                for (float weight : weights)
                {
                    if (isCloseTo(tolerance, weight, diff))
                    {
                        isTare = true;
                    }
                }
            }

            // set the last stable one to this one
            LOGI(TAG, "new last stable weight: %f\n", avgWeight);
            lastStableWeight = avgWeight;
        }
    }
}

bool AutoTare::shouldTare()
{
    if (isTare)
    {
        isTare = false;
        return true;
    }

    return false;
}