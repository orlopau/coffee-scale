#pragma once

#include "step.h"

#define RATIO_ADJUST_MULTIPLIER (RECIPE_RATIO_MUL / 10)

class RecipeConfigRatioStep : public RecipeStep
{
public:
    RecipeConfigRatioStep(RecipeStepState &state);
    void update() override;
    void enter() override;
    void exit() override;

private:
    RecipeStepState &state;
    uint32_t newRatio;
};
