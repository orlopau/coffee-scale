#pragma once

#include "battery.h"
#include "weighing/weight_source.h"
#include "millis.h"
#include "display.h"
#include <math.h>
#include <string.h>
#include <vector>

class MockWeightSource : public WeightSource
{
public:
    const std::vector<Reading> &newReadings() override { return readings; }
    float getWeight() override { return weight; }
    void tare() override
    {
        weight = 0;
        tares++;
    }
    uint32_t tareCount() override { return tares; }

    /** Makes grams the only new reading, at now(), and the weight. */
    void newReading(float grams, float flow = 0)
    {
        weight = grams;
        readings = {{static_cast<uint32_t>(now()), 0, grams, grams, grams, flow}};
    }

    float weight = 0;
    std::vector<Reading> readings;
    /** Tares so far; a test bumps it for a tare from elsewhere, e.g. lift-to-tare. */
    uint32_t tares = 0;
};
