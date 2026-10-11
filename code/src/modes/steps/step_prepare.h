#pragma once

#include "step.h"
#include "weighing/weight_source.h"

class RecipePrepare : public RecipeStep
{
public:
    RecipePrepare(RecipeStepState &state, WeightSource &weightSource);
    void update() override;
    void render(Canvas &canvas) override;
    void enter() override;
    void exit() override;
private:
    RecipeStepState &state;
    WeightSource &weightSource;
};