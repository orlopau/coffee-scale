#include <stdio.h>

#include "step_config_weight.h"
#include "interface.h"
#include "data/localization.h"
#include "ui/widgets.h"

RecipeConfigWeightStep::RecipeConfigWeightStep(RecipeStepState &state)
    : state(state){};

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

    state.configRecipe.coffeeWeightMg =
        state.originalRecipe->coffeeWeightMg + Interface::getEncoderTicks() * WEIGHT_ADJUST_MULTIPLIER;
}

unsigned int RecipeConfigWeightStep::getWaterWeightMl() const
{
    const Recipe &recipe = state.configRecipe;
    return recipe.coffeeWeightMg * ((float)recipeGetTotalRatio(recipe) / (float)RECIPE_RATIO_MUL) / 1000;
}

void RecipeConfigWeightStep::render(Canvas &canvas)
{
    static const int Y_PADDING = 4;
    static const int X_OFFSET = 10;

    const Recipe &recipe = state.configRecipe;

    int y = Widgets::titleLine(canvas, recipe.name) + Y_PADDING;
    int remainingHeight = canvas.height() - y;
    // labels are right aligned left of the center, values start at the center
    const double center = canvas.width() / 2.0;
    const int coffeeY = y + remainingHeight / 4.0;
    const int waterY = y + (remainingHeight / 4.0) * 3;
    char buffer[16];

    canvas.setFont(Font::SmallMedium);
    canvas.drawText(center - canvas.textWidth(DISPLAY_CONFIG_WEIGHT_COFFEE) - X_OFFSET,
                    Widgets::centerBaseline(canvas, coffeeY), DISPLAY_CONFIG_WEIGHT_COFFEE);
    if (Widgets::blinkVisible())
    {
        snprintf(buffer, sizeof(buffer), "%.1fg", recipe.coffeeWeightMg / 1000.0);
        canvas.setFont(Font::Medium);
        canvas.drawText(center, Widgets::centerBaseline(canvas, coffeeY), buffer);
    }

    canvas.setFont(Font::SmallMedium);
    canvas.drawText(center - canvas.textWidth(DISPLAY_CONFIG_WEIGHT_WATER) - X_OFFSET,
                    Widgets::centerBaseline(canvas, waterY), DISPLAY_CONFIG_WEIGHT_WATER);
    snprintf(buffer, sizeof(buffer), "%uml", getWaterWeightMl());
    canvas.setFont(Font::Medium);
    canvas.drawText(center, Widgets::centerBaseline(canvas, waterY), buffer);
}

void RecipeConfigWeightStep::enter() { Interface::resetEncoderTicks(); }
