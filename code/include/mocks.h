#pragma once

#include "battery.h"
#include "weight_sensor.h"
#include "millis.h"
#include "display.h"
#include <math.h>
#include <string.h>

class MockWeightSensor : public WeightSensor
{
public:
    float getWeight() override { return weight; }
    bool isNewWeight() override { return newWeight; }
    void tare() override { weight = 0; }
    void setScale(float scale) override {}
    float getLastWeight() override { return weight; }
    float getLastUntaredWeight() { return -1.0f; }
    float weight = 0;
    bool newWeight = false;
};