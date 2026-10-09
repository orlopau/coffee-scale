#pragma once

#include "step.h"

class RecipeSummaryStep : public RecipeStep
{
public:
    RecipeSummaryStep(RecipeStepState &state);
    void update() override {}
    void render(Canvas &canvas) override;

private:
    RecipeStepState &state;
};
