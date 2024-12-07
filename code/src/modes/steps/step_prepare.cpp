#include "step_prepare.h"
#include "interface.h"

RecipePrepare::RecipePrepare(RecipeStepState &state, Display &display, WeightSensor &weightSensor)
    : state(state), display(display), weightSensor(weightSensor) {}

void RecipePrepare::update()
{
    // tare on encoder rotate
    if (Interface::getEncoderDirection() != Interface::EncoderDirection::NONE)
    {
        weightSensor.tare();
    }

    display.recipeInsertCoffee(weightSensor.getWeight() * 1000, state.configRecipe.coffeeWeightMg);
}

void RecipePrepare::enter()
{
    weightSensor.tare();
}

void RecipePrepare::exit()
{
    weightSensor.tare();
}
