#pragma once

#include "step.h"
#include "scale.h"

class RecipePrepare : public RecipeStep
{
public:
    RecipePrepare(RecipeStepState &state, WeightSensor &weightSensor);
    void update() override;
    void enter() override;
    void exit() override;
private:
    RecipeStepState &state;
    WeightSensor &weightSensor;
};