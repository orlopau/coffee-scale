#pragma once

#include "scale.h"
#include "mode.h"
#include "stopwatch.h"
#include "display.h"
#include "regression.h"

#define ENCODER_MG_PER_TICK 100

#define MIN_TARGET_WEIGHT_MG 1000
#define MAX_TARGET_WEIGHT_MG 100000

#define REGRESSION_BUFFER_SIZE 50
#define REGRESSION_MAX_TIME 3 * 60 * 1000
#define REGRESSION_GRACE_PERIOD 1000

class ModeEspresso : public Mode
{
public:
    ModeEspresso(WeightSensor &weightSensor, Stopwatch &stopwatch)
        : weightSensor(weightSensor), stopwatch(stopwatch),
          targetWeightMg(36 * 1000), approximator(REGRESSION_BUFFER_SIZE), lastEstimatedTime(0){};
    ~ModeEspresso(){};
    void update() override;
    void enter() override;
    bool canSwitchMode() override;
    const char *getName() override;

private:
    WeightSensor &weightSensor;
    Stopwatch &stopwatch;

    Regression::Approximator approximator;
    int32_t targetWeightMg;
    long lastEstimatedTime;

    void handleNewWeight();
};