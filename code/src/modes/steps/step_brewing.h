#pragma once

#include "step.h"
#include "user_interface.h"
#include "loadcell.h"

class RecipeBrewing : public RecipeStep
{
public:
    RecipeBrewing(RecipeStepState &state, Display &display, UserInput &input, WeightSensor &weightSensor);
    void update() override;
    void enter() override;
    bool canStepForward() override;

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
    WeightSensor &weightSensor;

    enum BrewState
    {
        POUR,
        PAUSE,
        DONE,
    };
    unsigned long pourStartMillis = 0;
    uint8_t recipePourIndex;
    bool pourDoneFlag;
};
