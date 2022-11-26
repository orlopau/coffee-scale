#pragma once

#include "mode.h"
#include "recipe.h"

class ModeRecipes : public Mode
{
public:
    ~ModeRecipes();
    ModeRecipes(LoadCell &loadCell, UserInput &input,
                Display &display, const Recipe recipes[], uint8_t recipeCount);
    void update();
    uint8_t recipeIndex;
    uint8_t recipePourIndex;

    enum State
    {
        RECIPE_SELECTION = 0,
        RECIPE_SUMMARY,
        RECIPE_CONFIG,
        RECIPE_PREPARE,
        RECIPE_BREWING,
        RECIPE_DONE,
        SIZE,
    };
    State state;

private:
    LoadCell &loadCell;
    UserInput &input;
    Display &display;

    char *recipeSwitcherEntries;

    void updateRecipeSwitcher();
    void updateRecipeSummary();
    void updateRecipeConfig();
    void updateRecipePrepare();
    void updateRecipeBrewing();
    void updateRecipeDone();

    void updateSteps();

    const Recipe *recipes;
    uint8_t recipeCount;

    unsigned long pourStartMillis;
    int32_t coffeeWeightAdjustmentMg;
};
