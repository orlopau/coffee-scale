#pragma once

#include "step.h"
#include "ui/widgets.h"

class RecipeDone : public RecipeStep
{
public:
    RecipeDone(RecipeStepState &state) : state(state){};
    void update() override {}
    void render(Canvas &canvas) override
    {
        canvas.setFont(Font::Number30);
        // vertically centered: half the height plus half the font's ascent
        Widgets::textHCentered(canvas, "Done!", 32 + 15);
    }

private:
    RecipeStepState &state;
};
