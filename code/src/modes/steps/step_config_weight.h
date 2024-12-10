#pragma once

#include "step.h"

#define WEIGHT_ADJUST_MULTIPLIER 1000

class RecipeConfigWeightStep : public RecipeStep
{
public:
    RecipeConfigWeightStep(RecipeStepState &state);
    void update() override;
    void enter() override;

private:
    RecipeStepState &state;
};
