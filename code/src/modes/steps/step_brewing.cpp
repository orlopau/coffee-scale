#include "step_brewing.h"
#include "millis.h"

RecipeBrewing::RecipeBrewing(RecipeStepState &state, Display &display, UserInput &input, WeightSensor &weightSensor)
    : state(state), display(display), input(input), weightSensor(weightSensor) {}

void RecipeBrewing::update()
{
    const Pour *pour = &state.configRecipe.pours[recipePourIndex];

    uint64_t remainingTimePourMs;
    bool isPause = false;

    // autostart brew if enabled
    // extra if to start on same tick
    if (pourStartMillis == 0 && pour->autoStart)
    {
        pourStartMillis = now();
    }

    const uint64_t passedTimePourMs = now() - pourStartMillis;

    // Brew is not started yet.
    if (pourStartMillis == 0)
    {
        remainingTimePourMs = pour->timePour != 0 ? pour->timePour : pour->timePause;

        // if encoder is clicked, start brew
        if (input.getEncoderClick() == ClickType::SINGLE)
        {
            pourStartMillis = now();
        }
    }
    // Brew is in progress.
    else if (passedTimePourMs < pour->timePour)
    {
        remainingTimePourMs = pour->timePour - passedTimePourMs;
    }
    // Brew is in progress, pause time.
    else if (passedTimePourMs < pour->timePour + pour->timePause)
    {
        remainingTimePourMs = pour->timePour + pour->timePause - passedTimePourMs;
        isPause = true;
    }
    // Brew is done.
    else
    {
        remainingTimePourMs = 0;

        if (!pourDoneFlag)
        {
            pourDoneFlag = true;
            input.buzzerTone(200);
        }

        // if there is another pour, start it when encoder is clicked or auto advance is enabled
        if ((input.getEncoderClick() == ClickType::SINGLE || pour->autoAdvance) &&
            recipePourIndex + 1 < state.configRecipe.poursCount)
        {
            recipePourIndex++;
            pourStartMillis = 0;
            pourDoneFlag = false;
        }
    }

    const int32_t remainingWeightMg = (state.configRecipe.coffeeWeightMg * (pour->ratio / (float)RECIPE_RATIO_MUL)) -
                                      (weightSensor.getWeight() * 1000);
    display.recipePour(pour->note, remainingWeightMg, remainingTimePourMs, isPause, recipePourIndex,
                       state.configRecipe.poursCount);
}

void RecipeBrewing::enter()
{
    recipePourIndex = 0;
    pourStartMillis = 0;
    pourDoneFlag = false;
}

bool RecipeBrewing::canStepForward()
{
    return recipePourIndex + 1 >= state.configRecipe.poursCount && pourDoneFlag;
}
