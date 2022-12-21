#pragma once

#include "step.h"
#include "user_interface.h"
#include "loadcell.h"

class RecipePrepare : public RecipeStep
{
public:
    RecipePrepare(RecipeStepState &state, Display &display, UserInput &input, WeightSensor &weightSensor);
    void update() override;
    void enter() override;
    void exit() override;
private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
    WeightSensor &weightSensor;
};