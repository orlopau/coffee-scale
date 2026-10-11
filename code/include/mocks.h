#pragma once

#include "battery.h"
#include "weighing/weight_sensor.h"
#include "millis.h"
#include "display.h"
#include <math.h>
#include <string.h>
#include <vector>

class MockWeightSensor : public WeightSensor
{
public:
    const std::vector<Reading> &newReadings() override { return readings; }
    float getWeight() override { return weight; }
    void tare() override { weight = 0; }
    void setScale(float scale) override {}

    /** Makes grams the only new reading, at now(), and the weight. */
    void newReading(float grams)
    {
        weight = grams;
        readings = {{static_cast<uint32_t>(now()), 0, grams, grams, grams, 0}};
    }

    float weight = 0;
    std::vector<Reading> readings;
};
