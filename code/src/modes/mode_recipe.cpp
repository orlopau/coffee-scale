#include <cstring>

#include "mode_recipe.h"
#include "millis.h"

#include "modes/steps/step_switcher.h"
#include "modes/steps/step_summary.h"
#include "modes/steps/step_config_ratio.h"
#include "modes/steps/step_config_weight.h"
#include "modes/steps/step_prepare.h"
#include "modes/steps/step_brewing.h"
#include "modes/steps/step_done.h"

ModeRecipes::ModeRecipes(WeightSensor &weightSensor, UserInput &input, Display &display, const Recipe recipes[],
                         uint8_t recipeCount)
    : weightSensor(weightSensor), input(input), display(display), currentRecipeStep(0),
      recipeSteps{
          new RecipeSwitcherStep(recipeStepState, display, input, recipes, recipeCount),
          new RecipeSummaryStep(recipeStepState, display),
          new RecipeConfigRatioStep(recipeStepState, display, input),
          new RecipeConfigWeightStep(recipeStepState, display, input),
          new RecipePrepare(recipeStepState, display, input, weightSensor),
          new RecipeBrewing(recipeStepState, display, input, weightSensor),
          new RecipeDone(recipeStepState, display),
      } {};

void ModeRecipes::update()
{
    // update current step
    // checks if step is ready to advance, and if so, advances
    // if long click, go back to previous step
    // when all steos are done, advancing to next step returns to first step
    uint8_t previousStep = currentRecipeStep;
    if (input.getEncoderClick() == ClickType::SINGLE && recipeSteps[currentRecipeStep]->canStepForward())
    {
        currentRecipeStep++;
        if (currentRecipeStep >= recipeStepCount)
        {
            currentRecipeStep = 0;
        }
    }
    else if (input.getEncoderClick() == ClickType::LONG && recipeSteps[currentRecipeStep]->canStepBackward())
    {
        if (currentRecipeStep > 0)
        {
            currentRecipeStep--;
        }
    }

    if (previousStep != currentRecipeStep)
    {
        recipeSteps[previousStep]->exit();
        recipeSteps[currentRecipeStep]->enter();
        input.consumeEncoderClick();
    }

    recipeSteps[currentRecipeStep]->update();
}

bool ModeRecipes::canSwitchMode()
{
    return currentRecipeStep == 0;
}

uint8_t ModeRecipes::getCurrentStepIndex()
{
    return currentRecipeStep;
}

const char *ModeRecipes::getName()
{
    return "Recipes";
}