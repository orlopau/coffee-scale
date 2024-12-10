#pragma once

#include "step.h"

class RecipeSummaryStep : public RecipeStep
{
public:
    RecipeSummaryStep(RecipeStepState &state);
    void update() override;
    void enter() override;

private:
    RecipeStepState &state;
    bool isDisplayed;
};
