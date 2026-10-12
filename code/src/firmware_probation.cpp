#include "firmware_probation.h"

bool FirmwareProbation::update(uint32_t nowMs, bool readingsArrived)
{
    if (done)
    {
        return false;
    }
    if (!started)
    {
        started = true;
        startMs = nowMs;
    }
    sawReadings = sawReadings || readingsArrived;
    if (sawReadings && nowMs - startMs >= RUN_MS)
    {
        done = true;
        return true;
    }
    return false;
}
