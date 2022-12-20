#pragma once

#include "step.h"
#include "user_interface.h"

#define WEIGHT_ADJUST_MULTIPLIER 1000

class RecipeConfigWeightStep : public RecipeStep
{
public:
    RecipeConfigWeightStep(RecipeStepState &state, Display &display, UserInput &input);
    void update() override;
    void enter() override;

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
};
