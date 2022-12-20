#pragma once

#include "step.h"
#include "user_interface.h"

class RecipeSummaryStep : public RecipeStep
{
public:
    RecipeSummaryStep(RecipeStepState &state, Display &display);
    void update() override;

private:
    RecipeStepState &state;
    Display &display;
};
