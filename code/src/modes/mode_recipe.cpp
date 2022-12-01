#include <cstring>

#include "mode_recipe.h"
#include "millis.h"

#define WEIGHT_ADJUST_MULTIPLIER 1000

ModeRecipes::~ModeRecipes()
{
    delete[] recipeSwitcherEntries;
}

ModeRecipes::ModeRecipes(LoadCell &loadCell, UserInput &input, Display &display, const Recipe recipes[],
                         uint8_t recipeCount)
    : loadCell(loadCell), input(input), display(display), state(RECIPE_SELECTION), recipes(recipes),
      recipeCount(recipeCount), recipeIndex(0), recipePourIndex(0), pourStartMillis(0), coffeeWeightAdjustmentMg(0)
{
    recipeSwitcherEntries = new char[recipeCount * 64 + recipeCount * 2];
    strcpy(recipeSwitcherEntries, "\0");

    for (uint8_t i = 0; i < recipeCount; i++)
    {
        strncat(recipeSwitcherEntries, recipes[i].name, 64);
        if (i < recipeCount - 1)
        {
            strcat(recipeSwitcherEntries, "\n");
        }
    }
};

void ModeRecipes::update()
{
    updateSteps();

    switch (state)
    {
    case RECIPE_SELECTION:
        updateRecipeSwitcher();
        break;
    case RECIPE_SUMMARY:
        updateRecipeSummary();
        break;
    case RECIPE_CONFIG:
        updateRecipeConfig();
        break;
    case RECIPE_PREPARE:
        updateRecipePrepare();
        break;
    case RECIPE_BREWING:
        updateRecipeBrewing();
        break;
    case RECIPE_DONE:
        updateRecipeDone();
        break;
    }
}

void ModeRecipes::updateSteps()
{
    // long press returns, single press advances
    bool hasChanged = false;
    if (input.getEncoderClick() == ClickType::LONG && state != RECIPE_DONE)
    {
        if (static_cast<uint8_t>(state) > 0)
        {
            state = static_cast<State>(static_cast<uint8_t>(state) - 1);
            hasChanged = true;
        }
    }
    else if (input.getEncoderClick() == ClickType::SINGLE && state != RECIPE_BREWING && state != RECIPE_DONE)
    {
        // prevent forward state change when brewing is active
        if (static_cast<uint8_t>(state) + 1 < static_cast<uint8_t>(SIZE))
        {
            state = static_cast<State>(static_cast<uint8_t>(state) + 1);
            hasChanged = true;
        }
    }

    if (hasChanged)
    {
        switch (state)
        {
        case RECIPE_BREWING:
            loadCell.tare();
            recipePourIndex = 0;
            pourStartMillis = 0;
            pourStartMillis = now();
            break;
        }
    }
}

void ModeRecipes::updateRecipeSwitcher()
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

void ModeRecipes::updateRecipeSummary()
{
    display.recipeSummary(recipes[recipeIndex].name, recipes[recipeIndex].note);
}

void ModeRecipes::updateRecipeConfig()
{
    const Recipe *recipe = &recipes[recipeIndex];

    int lowerBoundTicks = -((recipe->coffeeWeightMg - 1) / WEIGHT_ADJUST_MULTIPLIER);
    int upperBoundTicks = 128;
    if (input.getEncoderTicks() > upperBoundTicks)
    {
        input.setEncoderTicks(upperBoundTicks);
    }
    else if (input.getEncoderTicks() < lowerBoundTicks)
    {
        input.setEncoderTicks(lowerBoundTicks);
    }

    coffeeWeightAdjustmentMg = input.getEncoderTicks() * WEIGHT_ADJUST_MULTIPLIER;

    const uint32_t coffeeWeightMg = recipe->coffeeWeightMg + coffeeWeightAdjustmentMg;
    const uint32_t waterWeight = coffeeWeightMg * (recipe->ratio / 100);
    display.recipeCoffeeWeightConfig(recipe->name, coffeeWeightMg, waterWeight);
}

void ModeRecipes::updateRecipePrepare()
{
    // display that user should insert coffee grounds
    static char buffer[64];
    sprintf(buffer, "Insert %dg coffee,\nthen click to\ncontinue.",
            (recipes[recipeIndex].coffeeWeightMg + coffeeWeightAdjustmentMg) / 1000);
    display.text(buffer);
}

enum BrewState
{
    POUR,
    PAUSE,
    DONE,
};

void ModeRecipes::updateRecipeBrewing()
{
    const Recipe *recipe = &recipes[recipeIndex];
    const Pour *pour = &recipe->pours[recipePourIndex];

    const uint32_t coffeeWeightMg = recipe->coffeeWeightMg + coffeeWeightAdjustmentMg;
    const int32_t remainingWeightMg = (coffeeWeightMg * (pour->ratio / 100)) - (loadCell.getWeight() * 1000);

    uint64_t passedTimeMs = now() - pourStartMillis;
    uint64_t remainingTimeMs;

    BrewState brewState;
    if (passedTimeMs < pour->timePour)
    {
        brewState = POUR;
        remainingTimeMs = pour->timePour - passedTimeMs;
    }
    else if (passedTimeMs < pour->timePour + pour->timePause)
    {
        brewState = PAUSE;
        remainingTimeMs = pour->timePour + pour->timePause - passedTimeMs;
    }
    else
    {
        brewState = DONE;
        remainingTimeMs = 0;
        if (input.getEncoderClick() == ClickType::SINGLE)
        {
            // if whole recipe is done, go to done, else go to next pour
            if (recipePourIndex + 1 < recipe->poursCount)
            {
                recipePourIndex++;
                pourStartMillis = now();
            }
            else
            {
                state = RECIPE_DONE;
            }
        }
    }

    display.recipePour(remainingWeightMg, remainingTimeMs, brewState == PAUSE, recipePourIndex, recipe->poursCount);
}

void ModeRecipes::updateRecipeDone()
{
    display.centerText("Done!", 30);
    if (input.getEncoderClick() == ClickType::SINGLE)
    {
        state = RECIPE_SELECTION;
    }
};

bool ModeRecipes::canSwitchMode()
{
    return state == RECIPE_SELECTION;
}

const char* ModeRecipes::getName()
{
    return "Recipes";
}