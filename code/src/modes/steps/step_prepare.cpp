#include <stdio.h>

#include "step_prepare.h"
#include "interface.h"
#include "data/localization.h"
#include "ui/widgets.h"

RecipePrepare::RecipePrepare(RecipeStepState &state, WeightSensor &weightSensor)
    : state(state), weightSensor(weightSensor) {}

void RecipePrepare::update()
{
    // tare on encoder rotate
    if (Interface::getEncoderDirection() != Interface::EncoderDirection::NONE)
    {
        weightSensor.tare();
    }
}

void RecipePrepare::render(Canvas &canvas)
{
    canvas.setFont(Font::Mono13);
    Widgets::textHCentered(canvas, DISPLAY_INSERT_COFFEE, canvas.ascent() + 5);

    int32_t weightMg = weightSensor.getWeight() * 1000;
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%.2fg/%.1fg", weightMg / 1000.0, state.configRecipe.coffeeWeightMg / 1000.0);
    canvas.setFont(Font::Mono18);
    Widgets::textCentered(canvas, buffer);
}

void RecipePrepare::enter()
{
    weightSensor.tare();
}

void RecipePrepare::exit()
{
    weightSensor.tare();
}
