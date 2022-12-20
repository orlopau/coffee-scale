#include "step_config_weight.h"

RecipeConfigWeightStep::RecipeConfigWeightStep(RecipeStepState &state, Display &display, UserInput &input) : state(state), display(display), input(input){};

void RecipeConfigWeightStep::update()
{
    // declare bounds for adjustment
    int lowerBoundTicks, upperBoundTicks;
    lowerBoundTicks = -((state.originalRecipe->coffeeWeightMg - 1) / WEIGHT_ADJUST_MULTIPLIER);
    upperBoundTicks = 128;

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
    state.configRecipe.coffeeWeightMg = state.originalRecipe->coffeeWeightMg + input.getEncoderTicks() * WEIGHT_ADJUST_MULTIPLIER;
    display.recipeConfigCoffeeWeight(state.configRecipe.name, state.configRecipe.coffeeWeightMg,
                                     state.configRecipe.coffeeWeightMg * ((float)state.configRecipe.ratio / (float)RECIPE_RATIO_MUL) / 1000);
}

void RecipeConfigWeightStep::enter()
{
    input.resetEncoderTicks();
}
