#pragma once

#include "mode.h"
#include "recipe.h"
#include "modes/steps/step.h"
#include "loadcell.h"
#include "user_interface.h"

class ModeRecipes : public Mode
{
public:
    ModeRecipes(WeightSensor &weightSensor, UserInput &input,
                Display &display, const Recipe recipes[], uint8_t recipeCount);
    void update();
    const char *getName();
    bool canSwitchMode();
    uint8_t getCurrentStepIndex();

private:
    WeightSensor &weightSensor;
    UserInput &input;
    Display &display;

    RecipeStepState recipeStepState;

    uint8_t currentRecipeStep;
    static const uint8_t recipeStepCount = 7;
    RecipeStep *recipeSteps[recipeStepCount];
};
