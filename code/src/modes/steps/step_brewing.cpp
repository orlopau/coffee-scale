#include "step_brewing.h"
#include "millis.h"

RecipeBrewing::RecipeBrewing(RecipeStepState &state, Display &display, UserInput &input, WeightSensor &weightSensor)
    : state(state), display(display), input(input), weightSensor(weightSensor) {}

void RecipeBrewing::update()
{
    const Pour *pour = &state.configRecipe.pours[recipePourIndex];

    const int32_t remainingWeightMg = (state.configRecipe.coffeeWeightMg * (pour->ratio / (float)RECIPE_RATIO_MUL)) - (weightSensor.getWeight() * 1000);
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
        if (!pourDoneFlag)
        {
            pourDoneFlag = true;
            input.buzzerTone(200);
        }
        brewState = DONE;
        remainingTimePourMs = 0;
        // if there is another pour, start it
        if (input.getEncoderClick() == ClickType::SINGLE && recipePourIndex + 1 < state.configRecipe.poursCount)
        {
            recipePourIndex++;
            pourStartMillis = now();
            pourDoneFlag = false;
        }
    }

    display.recipePour(pour->note, remainingWeightMg, remainingTimePourMs, brewState == PAUSE, recipePourIndex, state.configRecipe.poursCount);
}

void RecipeBrewing::enter()
{
    recipePourIndex = 0;
    pourStartMillis = now();
    pourDoneFlag = false;
}

bool RecipeBrewing::canStepForward()
{
    return recipePourIndex + 1 >= state.configRecipe.poursCount;
}
