#pragma once

#include "recipe.h"

struct RecipeStepState
{
    const Recipe *originalRecipe;
    Recipe configRecipe;
};

class RecipeStep
{
public:
    virtual ~RecipeStep(){};
    virtual void update() = 0;
    virtual void enter(){};
    virtual void exit(){};
    virtual bool canStepForward() { return true; };
    virtual bool canStepBackward() { return true; };
};