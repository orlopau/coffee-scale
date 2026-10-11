#include "input/detent_counter.h"

int32_t DetentCounter::update(int32_t counts)
{
    while (counts - detent * COUNTS_PER_DETENT >= THRESHOLD)
    {
        detent++;
    }
    while (counts - detent * COUNTS_PER_DETENT <= -THRESHOLD)
    {
        detent--;
    }
    return detent;
}
