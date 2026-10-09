#pragma once

#include "step.h"

#define RATIO_ADJUST_MULTIPLIER (RECIPE_RATIO_MUL / 10)

class RecipeConfigRatioStep : public RecipeStep
{
public:
    RecipeConfigRatioStep(RecipeStepState &state);
    void update() override;
    void render(Canvas &canvas) override;
    void enter() override;
    void exit() override;

    /** Configured total water to coffee ratio, multiplied by RECIPE_RATIO_MUL. */
    uint32_t getRatio() const { return newRatio; }

private:
    RecipeStepState &state;
    uint32_t newRatio = 0;
};
