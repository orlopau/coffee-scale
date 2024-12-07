#pragma once

#include "step.h"
#include "display.h"

class RecipeSwitcherStep : public RecipeStep
{
public:
    ~RecipeSwitcherStep();
    RecipeSwitcherStep(RecipeStepState &state, Display &display, const Recipe recipes[],
                       const uint8_t recipeCount);
    void update() override;
    void exit() override;
    uint8_t recipeIndex;

private:
    RecipeStepState &state;
    Display &display;

    const Recipe *recipes;
    const char **recipeSwitcherEntries;
    uint8_t recipeCount;
};
