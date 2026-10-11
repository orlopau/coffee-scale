#include <stdio.h>

#include "step_prepare.h"
#include "interface.h"
#include "data/localization.h"
#include "formatters.h"
#include "ui/widgets.h"

RecipePrepare::RecipePrepare(RecipeStepState &state, WeightSource &weightSource)
    : state(state), weightSource(weightSource) {}

void RecipePrepare::update()
{
    // tare on encoder rotate
    if (Interface::getEncoderTurns() != 0)
    {
        weightSource.tare();
    }
}

void RecipePrepare::render(Canvas &canvas)
{
    canvas.setFont(Font::Mono13);
    Widgets::textHCentered(canvas, DISPLAY_INSERT_COFFEE, canvas.ascent() + 5);

    int32_t weightMg = weightSource.getWeight() * 1000;
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%s/%.1fg", formatWeight(weightMg / 1000.0), state.configRecipe.coffeeWeightMg / 1000.0);
    canvas.setFont(Font::Mono18);
    Widgets::textCentered(canvas, buffer);
}

void RecipePrepare::enter()
{
    weightSource.tare();
}

void RecipePrepare::exit()
{
    weightSource.tare();
}
