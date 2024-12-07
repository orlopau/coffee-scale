#include "step_config_weight.h"
#include "interface.h"

RecipeConfigWeightStep::RecipeConfigWeightStep(RecipeStepState &state, Display &display)
    : state(state), display(display){};

void RecipeConfigWeightStep::update()
{
    // declare bounds for adjustment
    int lowerBoundTicks, upperBoundTicks;
    lowerBoundTicks = -((state.originalRecipe->coffeeWeightMg - 1) / WEIGHT_ADJUST_MULTIPLIER);
    upperBoundTicks = 128;

    // enforce bounds
    if (Interface::getEncoderTicks() > upperBoundTicks)
    {
        Interface::setEncoderTicks(upperBoundTicks);
    }
    else if (Interface::getEncoderTicks() < lowerBoundTicks)
    {
        Interface::setEncoderTicks(lowerBoundTicks);
    }

    // update values and display
    state.configRecipe.coffeeWeightMg =
        state.originalRecipe->coffeeWeightMg + Interface::getEncoderTicks() * WEIGHT_ADJUST_MULTIPLIER;
    display.recipeConfigCoffeeWeight(state.configRecipe.name, state.configRecipe.coffeeWeightMg,
                                     state.configRecipe.coffeeWeightMg *
                                         ((float)recipeGetTotalRatio(state.configRecipe) / (float)RECIPE_RATIO_MUL) / 1000);
}

void RecipeConfigWeightStep::enter() { Interface::resetEncoderTicks(); }
