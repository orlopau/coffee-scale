#include <cstring>

#include "mode_recipe.h"
#include "millis.h"

#define WEIGHT_ADJUST_MULTIPLIER 1000
#define RATIO_ADJUST_MULTIPLIER (RECIPE_RATIO_MUL / 10)

class RecipeSwitcherStep : public RecipeStep
{
public:
    ~RecipeSwitcherStep()
    {
        delete[] recipeSwitcherEntries;
    }

    RecipeSwitcherStep(RecipeStepState &state, Display &display, UserInput &input, const Recipe recipes[], const uint8_t recipeCount)
        : state(state), display(display), input(input), recipes(recipes), recipeCount(recipeCount), recipeIndex(0)
    {
        recipeSwitcherEntries = new const char *[recipeCount];
        for (uint8_t i = 0; i < recipeCount; i++)
        {
            recipeSwitcherEntries[i] = recipes[i].name;
        }
    }

    void update() override
    {
        int16_t change = static_cast<int16_t>(input.getEncoderDirection());

        if (recipeIndex + change >= recipeCount)
        {
            recipeIndex = recipeCount - 1;
        }
        else if (recipeIndex + change < 0)
        {
            recipeIndex = 0;
        }
        else
        {
            recipeIndex += change;
        }

        display.switcher(recipes[recipeIndex].name, recipeIndex, recipeCount, recipeSwitcherEntries);
    }

    void exit() override
    {
        state.originalRecipe = &recipes[recipeIndex];
        state.configRecipe = recipes[recipeIndex];
    }

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;

    const Recipe *recipes;
    const char **recipeSwitcherEntries;
    uint8_t recipeCount;
    uint8_t recipeIndex;
};

class RecipeSummaryStep : public RecipeStep
{
public:
    RecipeSummaryStep(RecipeStepState &state, Display &display) : state(state), display(display){};
    void update() override
    {
        display.recipeSummary(state.originalRecipe->name, state.originalRecipe->note);
    }

private:
    RecipeStepState &state;
    Display &display;
};

class RecipeConfigRatioStep : public RecipeStep
{
public:
    RecipeConfigRatioStep(RecipeStepState &state, Display &display, UserInput &input) : state(state), display(display), input(input){};
    void update() override
    {
        // declare bounds for adjustment
        int lowerBoundTicks, upperBoundTicks;
        lowerBoundTicks = -(state.originalRecipe->ratio - RECIPE_RATIO_MUL) / RATIO_ADJUST_MULTIPLIER;
        upperBoundTicks = 64 * RECIPE_RATIO_MUL;

        // enforce bounds
        if (input.getEncoderTicks() > upperBoundTicks)
        {
            input.setEncoderTicks(upperBoundTicks);
        }
        else if (input.getEncoderTicks() < lowerBoundTicks)
        {
            input.setEncoderTicks(lowerBoundTicks);
        }

        // update values and display
        state.configRecipe.ratio = state.originalRecipe->ratio + input.getEncoderTicks() * RATIO_ADJUST_MULTIPLIER;
        display.recipeConfigRatio(state.configRecipe.name, 1.0, (float)state.configRecipe.ratio / (float)RECIPE_RATIO_MUL);
    }
    void enter() override
    {
        input.resetEncoderTicks();
    }
    void exit() override
    {
        // adjust ratiuos of pours according to new ratio
        float ratioMultiplier = (float)state.configRecipe.ratio / (float)state.originalRecipe->ratio;
        for (uint8_t i = 0; i < state.configRecipe.poursCount; i++)
        {
            state.configRecipe.pours[i].ratio *= ratioMultiplier;
        }
    }

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
};

class RecipeConfigWeightStep : public RecipeStep
{
public:
    RecipeConfigWeightStep(RecipeStepState &state, Display &display, UserInput &input) : state(state), display(display), input(input){};
    void update() override
    {
        // declare bounds for adjustment
        int lowerBoundTicks, upperBoundTicks;
        lowerBoundTicks = -((state.originalRecipe->coffeeWeightMg - 1) / WEIGHT_ADJUST_MULTIPLIER);
        upperBoundTicks = 128;

        // enforce bounds
        if (input.getEncoderTicks() > upperBoundTicks)
        {
            input.setEncoderTicks(upperBoundTicks);
        }
        else if (input.getEncoderTicks() < lowerBoundTicks)
        {
            input.setEncoderTicks(lowerBoundTicks);
        }

        // update values and display
        state.configRecipe.coffeeWeightMg = state.originalRecipe->coffeeWeightMg + input.getEncoderTicks() * WEIGHT_ADJUST_MULTIPLIER;
        display.recipeCoffeeWeightConfig(state.configRecipe.name, state.configRecipe.coffeeWeightMg,
                                         state.configRecipe.coffeeWeightMg * ((float)state.configRecipe.ratio / (float)RECIPE_RATIO_MUL) / 1000);
    }
    void enter() override
    {
        input.resetEncoderTicks();
    }

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
};

class RecipePrepare : public RecipeStep
{
public:
    RecipePrepare(RecipeStepState &state, Display &display, LoadCell &loadCell) : state(state), display(display), loadCell(loadCell){};
    void update() override
    {
        // display that user should insert coffee grounds
        static char buffer[64];
        sprintf(buffer, "Insert %dg coffee,\nthen click to\ncontinue.", state.configRecipe.coffeeWeightMg / 1000);
        display.text(buffer);
    }
    void exit() override
    {
        loadCell.tare();
    }

private:
    RecipeStepState &state;
    Display &display;
    LoadCell &loadCell;
};

class RecipeBrewing : public RecipeStep
{
public:
    RecipeBrewing(RecipeStepState &state, Display &display, UserInput &input, LoadCell &loadCell)
        : state(state), display(display), input(input), loadCell(loadCell){};
    void update() override
    {
        const Pour *pour = &state.configRecipe.pours[recipePourIndex];

        const int32_t remainingWeightMg = (state.configRecipe.coffeeWeightMg * (pour->ratio / (float)RECIPE_RATIO_MUL)) - (loadCell.getWeight() * 1000);
        const uint64_t passedTimePourMs = now() - pourStartMillis;
        uint64_t remainingTimePourMs;

        BrewState brewState;
        if (passedTimePourMs < pour->timePour)
        {
            brewState = POUR;
            remainingTimePourMs = pour->timePour - passedTimePourMs;
        }
        else if (passedTimePourMs < pour->timePour + pour->timePause)
        {
            brewState = PAUSE;
            remainingTimePourMs = pour->timePour + pour->timePause - passedTimePourMs;
        }
        else
        {
            brewState = DONE;
            remainingTimePourMs = 0;
            // if there is another pour, start it
            if (input.getEncoderClick() == ClickType::SINGLE && recipePourIndex + 1 < state.configRecipe.poursCount)
            {
                recipePourIndex++;
                pourStartMillis = now();
            }
        }

        display.recipePour(pour->note, remainingWeightMg, remainingTimePourMs, brewState == PAUSE, recipePourIndex, state.configRecipe.poursCount);
    }
    void enter() override
    {
        recipePourIndex = 0;
        pourStartMillis = now();
    }
    bool canStepForward() override
    {
        return recipePourIndex + 1 >= state.configRecipe.poursCount;
    }

private:
    RecipeStepState &state;
    Display &display;
    UserInput &input;
    LoadCell &loadCell;

    enum BrewState
    {
        POUR,
        PAUSE,
        DONE,
    };
    unsigned long pourStartMillis = 0;
    uint8_t recipePourIndex = 0;
};

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

ModeRecipes::ModeRecipes(LoadCell &loadCell, UserInput &input, Display &display, const Recipe recipes[],
                         uint8_t recipeCount)
    : loadCell(loadCell), input(input), display(display), currentRecipeStep(0),
      recipeSteps{
          new RecipeSwitcherStep(recipeStepState, display, input, recipes, recipeCount),
          new RecipeSummaryStep(recipeStepState, display),
          new RecipeConfigRatioStep(recipeStepState, display, input),
          new RecipeConfigWeightStep(recipeStepState, display, input),
          new RecipePrepare(recipeStepState, display, loadCell),
          new RecipeBrewing(recipeStepState, display, input, loadCell),
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