#include "step_summary.h"
#include "display.h"

RecipeSummaryStep::RecipeSummaryStep(RecipeStepState &state) : state(state), isDisplayed(false) {}

void RecipeSummaryStep::update()
{
    if (isDisplayed)
    {
        return;
    }

    Display::recipeSummary(state.originalRecipe->name, state.originalRecipe->note, state.originalRecipe->url[0] == '\0' ? nullptr : state.originalRecipe->url);
    isDisplayed = true;
}

void RecipeSummaryStep::enter() { isDisplayed = false; }