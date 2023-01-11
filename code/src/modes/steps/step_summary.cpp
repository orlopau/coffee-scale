#include "step_summary.h"

RecipeSummaryStep::RecipeSummaryStep(RecipeStepState &state, Display &display) : state(state), display(display), isDisplayed(false) {}

void RecipeSummaryStep::update()
{
    if (isDisplayed)
    {
        return;
    }

    display.recipeSummary(state.originalRecipe->name, state.originalRecipe->note, state.originalRecipe->url[0] == '\0' ? nullptr : state.originalRecipe->url);
    isDisplayed = true;
}

void RecipeSummaryStep::enter() { isDisplayed = false; }