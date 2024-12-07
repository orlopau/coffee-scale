#pragma once

#include "step.h"
#include "display.h"
#include "scale.h"

class RecipePrepare : public RecipeStep
{
public:
    RecipePrepare(RecipeStepState &state, Display &display, WeightSensor &weightSensor);
    void update() override;
    void enter() override;
    void exit() override;
private:
    RecipeStepState &state;
    Display &display;
    WeightSensor &weightSensor;
};