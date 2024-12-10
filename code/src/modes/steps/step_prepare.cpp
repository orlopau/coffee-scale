#include "step_prepare.h"
#include "interface.h"
#include "display.h"

RecipePrepare::RecipePrepare(RecipeStepState &state, WeightSensor &weightSensor)
    : state(state), weightSensor(weightSensor) {}

void RecipePrepare::update()
{
    // tare on encoder rotate
    if (Interface::getEncoderDirection() != Interface::EncoderDirection::NONE)
    {
        weightSensor.tare();
    }

    Display::recipeInsertCoffee(weightSensor.getWeight() * 1000, state.configRecipe.coffeeWeightMg);
}

void RecipePrepare::enter()
{
    weightSensor.tare();
}

void RecipePrepare::exit()
{
    weightSensor.tare();
}
