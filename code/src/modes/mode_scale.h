#pragma once

#include "auto_tare.h"
#include "mode.h"
#include "stopwatch.h"
#include "weight_sensor.h"

class ModeScale : public Mode
{
public:
    ModeScale(WeightSensor &weightSensor, Stopwatch &stopwatch)
        : weightSensor(weightSensor), stopwatch(stopwatch){};
    ~ModeScale(){};
    void update();
    bool canSwitchMode();
    const char* getName();

private:
    WeightSensor &weightSensor;
    Stopwatch &stopwatch;
    AutoTare *autoTare = new AutoTare(0.1, 1, 16); // 1g std deviation
};