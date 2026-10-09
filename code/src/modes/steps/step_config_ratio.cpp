#include <stdio.h>

#include "step_config_ratio.h"
#include "interface.h"
#include "data/localization.h"
#include "ui/widgets.h"

RecipeConfigRatioStep::RecipeConfigRatioStep(RecipeStepState &state)
    : state(state)
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
}

void RecipeConfigRatioStep::render(Canvas &canvas)
{
    static const int Y_PADDING = 4;

    const int width = canvas.width();
    int y = Widgets::titleLine(canvas, state.configRecipe.name);
    y += 2 * Y_PADDING;

    canvas.setFont(Font::Medium);
    y += canvas.ascent();
    Widgets::textHCentered(canvas, DISPLAY_CONFIG_RATIO, y);
    y += Y_PADDING;

    // coffee : water, with the edited water part blinking
    canvas.setFont(Font::Large);
    y += (canvas.height() - y) / 2.0 + canvas.ascent() / 2.0;
    canvas.drawText(width / 2.0 - canvas.textWidth(":") / 2.0, y, ":");

    canvas.drawText(width / 4.0 - canvas.textWidth("1.0") / 2.0, y, "1.0");
    if (Widgets::blinkVisible())
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", newRatio / (double)RECIPE_RATIO_MUL);
        canvas.drawText(3 * width / 4.0 - canvas.textWidth(buffer) / 2.0, y, buffer);
    }
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
