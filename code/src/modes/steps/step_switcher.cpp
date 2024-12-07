#include "step_switcher.h"
#include "data/localization.h"
#include "interface.h"

RecipeSwitcherStep::~RecipeSwitcherStep()
{
    delete[] recipeSwitcherEntries;
}

RecipeSwitcherStep::RecipeSwitcherStep(RecipeStepState &state, Display &display, const Recipe recipes[], const uint8_t recipeCount)
    : state(state), display(display), recipes(recipes), recipeCount(recipeCount), recipeIndex(0)
{
    recipeSwitcherEntries = new const char *[recipeCount];
    for (uint8_t i = 0; i < recipeCount; i++)
    {
        recipeSwitcherEntries[i] = recipes[i].name;
    }
}

void RecipeSwitcherStep::update()
{
    int16_t change = static_cast<int16_t>(Interface::getEncoderDirection());

    if (recipeIndex + change >= recipeCount)
    {
        recipeIndex = recipeCount - 1;
    }
    else if (recipeIndex + change < 0)
    {
        recipeIndex = 0;
    }
    else
    {
        recipeIndex += change;
    }

    display.switcher(DISPLAY_RECIPE_SWITCHER_TITLE, recipeIndex, recipeCount, recipeSwitcherEntries);
}

void RecipeSwitcherStep::exit()
{
    state.originalRecipe = &recipes[recipeIndex];
    state.configRecipe = recipes[recipeIndex];
}
