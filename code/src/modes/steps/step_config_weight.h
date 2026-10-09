#pragma once

#include "step.h"

#define WEIGHT_ADJUST_MULTIPLIER 1000

class RecipeConfigWeightStep : public RecipeStep
{
public:
    RecipeConfigWeightStep(RecipeStepState &state);
    void update() override;
    void render(Canvas &canvas) override;
    void enter() override;

    /** Water for the configured coffee weight, in ml. */
    unsigned int getWaterWeightMl() const;

private:
    RecipeStepState &state;
};
