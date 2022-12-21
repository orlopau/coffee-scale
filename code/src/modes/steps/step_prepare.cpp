#include "step_prepare.h"

RecipePrepare::RecipePrepare(RecipeStepState &state, Display &display, UserInput &input, WeightSensor &weightSensor)
    : state(state), input(input), display(display), weightSensor(weightSensor) {}

void RecipePrepare::update()
{
    // tare on encoder rotate
    if (input.getEncoderDirection() != EncoderDirection::NONE)
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
