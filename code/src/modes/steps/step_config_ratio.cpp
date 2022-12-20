#include "step_config_ratio.h"

RecipeConfigRatioStep::RecipeConfigRatioStep(RecipeStepState &state, Display &display, UserInput &input) : state(state), display(display), input(input) {}

void RecipeConfigRatioStep::update()
{
    // declare bounds for adjustment
    int lowerBoundTicks, upperBoundTicks;
    lowerBoundTicks = -(state.originalRecipe->ratio - RECIPE_RATIO_MUL) / RATIO_ADJUST_MULTIPLIER;
    upperBoundTicks = 64 * RECIPE_RATIO_MUL;

    // enforce bounds
    if (input.getEncoderTicks() > upperBoundTicks)
    {
        input.setEncoderTicks(upperBoundTicks);
    }
    else if (input.getEncoderTicks() < lowerBoundTicks)
    {
        input.setEncoderTicks(lowerBoundTicks);
    }

    // update values and display
    state.configRecipe.ratio = state.originalRecipe->ratio + input.getEncoderTicks() * RATIO_ADJUST_MULTIPLIER;
    display.recipeConfigRatio(state.configRecipe.name, 1.0, (float)state.configRecipe.ratio / (float)RECIPE_RATIO_MUL);
}

void RecipeConfigRatioStep::enter()
{
    input.resetEncoderTicks();
}

void RecipeConfigRatioStep::exit()
{
    // adjust ratiuos of pours according to new ratio
    float ratioMultiplier = (float)state.configRecipe.ratio / (float)state.originalRecipe->ratio;
    for (uint8_t i = 0; i < state.configRecipe.poursCount; i++)
    {
        state.configRecipe.pours[i].ratio *= ratioMultiplier;
    }
}
