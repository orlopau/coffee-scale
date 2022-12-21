#include "step_summary.h"

RecipeSummaryStep::RecipeSummaryStep(RecipeStepState &state, Display &display) : state(state), display(display) {}

void RecipeSummaryStep::update()
{
    display.recipeSummary(state.originalRecipe->name, state.originalRecipe->note);
}