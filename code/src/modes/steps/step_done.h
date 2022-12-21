#pragma once

#include "step.h"
#include "user_interface.h"

class RecipeDone : public RecipeStep
{
public:
    RecipeDone(RecipeStepState &state, Display &display) : state(state), display(display){};
    void update() override
    {
        display.centerText("Done!", 30);
    }

private:
    RecipeStepState &state;
    Display &display;
};
