#pragma once

#include "step.h"
#include "display.h"

class RecipeDone : public RecipeStep
{
public:
    RecipeDone(RecipeStepState &state) : state(state){};
    void update() override
    {
        Display::centerText("Done!", 30);
    }

private:
    RecipeStepState &state;
};
