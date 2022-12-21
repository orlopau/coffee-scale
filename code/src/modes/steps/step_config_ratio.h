#pragma once

#include "step.h"
#include "user_interface.h"

#define RATIO_ADJUST_MULTIPLIER (RECIPE_RATIO_MUL / 10)

class RecipeConfigRatioStep : public RecipeStep
{
public:
    RecipeConfigRatioStep(RecipeStepState &state, Display &display, UserInput &input);
    void update() override;
    void enter() override;
    void exit() override;

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
};
