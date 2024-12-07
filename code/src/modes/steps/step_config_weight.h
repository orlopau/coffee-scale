#pragma once

#include "step.h"
#include "display.h"

#define WEIGHT_ADJUST_MULTIPLIER 1000

class RecipeConfigWeightStep : public RecipeStep
{
public:
    RecipeConfigWeightStep(RecipeStepState &state, Display &display);
    void update() override;
    void enter() override;

private:
    RecipeStepState &state;
    Display &display;
};
