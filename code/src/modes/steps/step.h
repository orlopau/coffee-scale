#pragma once

#include "recipe.h"
#include "canvas.h"

struct RecipeStepState
{
    const Recipe *originalRecipe;
    Recipe configRecipe;
};

class RecipeStep
{
public:
    virtual ~RecipeStep(){};
    /** Handles input and state, must not draw. */
    virtual void update() = 0;
    /** Draws the current state, see Mode::render(). */
    virtual void render(Canvas &canvas) = 0;
    virtual void enter(){};
    virtual void exit(){};
    virtual bool canStepForward() { return true; };
    virtual bool canStepBackward() { return true; };
};