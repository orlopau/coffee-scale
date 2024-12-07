#include "step_config_ratio.h"
#include "interface.h"

RecipeConfigRatioStep::RecipeConfigRatioStep(RecipeStepState &state, Display &display)
    : state(state), display(display)
{
}

void RecipeConfigRatioStep::update()
{
    // declare bounds for adjustment
    int lowerBoundTicks, upperBoundTicks;
    lowerBoundTicks = -(recipeGetTotalRatio(*state.originalRecipe) - RECIPE_RATIO_MUL) / RATIO_ADJUST_MULTIPLIER;
    upperBoundTicks = 64 * RECIPE_RATIO_MUL;

    // enforce bounds
    if (Interface::getEncoderTicks() > upperBoundTicks)
    {
        Interface::setEncoderTicks(upperBoundTicks);
    }
    else if (Interface::getEncoderTicks() < lowerBoundTicks)
    {
        Interface::setEncoderTicks(lowerBoundTicks);
    }

    // adjust ratios of pours according to new ratio
    newRatio = recipeGetTotalRatio(*state.originalRecipe) + Interface::getEncoderTicks() * RATIO_ADJUST_MULTIPLIER;

    // update values and display
    display.recipeConfigRatio(state.configRecipe.name, 1 * RECIPE_RATIO_MUL, newRatio);
}

void RecipeConfigRatioStep::enter()
{
    // reset ratio to default
    Interface::resetEncoderTicks();
    for (uint8_t i = 0; i < state.configRecipe.poursCount; i++)
    {
        state.configRecipe.pours[i].ratio = state.originalRecipe->pours[i].ratio;
    }
}

void RecipeConfigRatioStep::exit()
{
    // adjust ratios of pours according to new ratio
    float ratioMultiplier = newRatio / (float)recipeGetTotalRatio(*state.originalRecipe);
    for (uint8_t i = 0; i < state.configRecipe.poursCount; i++)
    {
        state.configRecipe.pours[i].ratio *= ratioMultiplier;
    }
}
