#pragma once

#include "mode.h"
#include "recipe.h"
#include "modes/steps/step.h"
#include "weighing/weight_source.h"

class ModeRecipes : public Mode
{
public:
    ModeRecipes(WeightSource &weightSource, const Recipe recipes[], uint8_t recipeCount);
    void update() override;
    void render(Canvas &canvas) override;
    const char *getName();
    bool canSwitchMode();
    uint8_t getCurrentStepIndex();

private:
    WeightSource &weightSource;

    RecipeStepState recipeStepState;

    uint8_t currentRecipeStep;
    static const uint8_t recipeStepCount = 7;
    RecipeStep *recipeSteps[recipeStepCount];
};
