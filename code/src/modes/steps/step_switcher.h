#pragma once

#include "step.h"
#include "user_interface.h"

class RecipeSwitcherStep : public RecipeStep
{
public:
    ~RecipeSwitcherStep();
    RecipeSwitcherStep(RecipeStepState &state, Display &display, UserInput &input, const Recipe recipes[], const uint8_t recipeCount);
    void update() override;
    void exit() override;
private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;

    const Recipe *recipes;
    const char **recipeSwitcherEntries;
    uint8_t recipeCount;
    uint8_t recipeIndex;
};
