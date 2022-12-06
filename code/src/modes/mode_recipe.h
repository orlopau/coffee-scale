#pragma once

#include "mode.h"
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

class ModeRecipes : public Mode
{
public:
    ModeRecipes(LoadCell &loadCell, UserInput &input,
                Display &display, const Recipe recipes[], uint8_t recipeCount);
    void update();
    const char *getName();
    bool canSwitchMode();
    uint8_t getCurrentStepIndex();

private:
    LoadCell &loadCell;
    UserInput &input;
    Display &display;

    RecipeStepState recipeStepState;

    uint8_t currentRecipeStep;
    static const uint8_t recipeStepCount = 7;
    RecipeStep *recipeSteps[recipeStepCount];
};
