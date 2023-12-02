#pragma once

#include "loadcell.h"
#include "mode.h"
#include "util/stopwatch.h"
#include "user_interface.h"
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
    ModeEspresso(WeightSensor &weightSensor, UserInput &buttons, Display &display, Stopwatch &stopwatch)
        : weightSensor(weightSensor), buttons(buttons), display(display), stopwatch(stopwatch),
          targetWeightMg(36 * 1000), approximator(REGRESSION_BUFFER_SIZE), lastEstimatedTime(0){};
    ~ModeEspresso(){};
    void update() override;
    void enter() override;
    bool canSwitchMode() override;
    const char *getName() override;

private:
    WeightSensor &weightSensor;
    UserInput &buttons;
    Display &display;
    Stopwatch &stopwatch;

    Regression::Approximator approximator;
    int32_t targetWeightMg;
    long lastEstimatedTime;

    void handleNewWeight();
};