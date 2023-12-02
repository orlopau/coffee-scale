#include "step_brewing.h"
#include "util/millis.h"

RecipeBrewing::RecipeBrewing(RecipeStepState &state, Display &display, UserInput &input, WeightSensor &weightSensor)
    : state(state), display(display), input(input), weightSensor(weightSensor)
{
}

void RecipeBrewing::update()
{
    const Pour *pour = &state.configRecipe.pours[recipePourIndex];

    uint64_t remainingTimePourMs;
    bool isPause = false;

    // tare scale on rotation
    if (input.getEncoderDirection() != EncoderDirection::NONE)
    {
        weightSensor.tare();
    }

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
            // consume, else we will advance to next pour in a later if
            input.consumeEncoderClick();
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

        // if there is another pour, start when auto advance is on
        if (pour->autoAdvance)
        {
            nextPour();
        }
    }

    // if brew has started, clicking should advance to next pour
    if (pourStartMillis != 0 && input.getEncoderClick() == ClickType::SINGLE)
    {
        nextPour();
    }

    // calculate remaining weight, by adding the weight of all pours including the current one
    int32_t totalPourWeightMg = 0;
    for (int i = 0; i <= recipePourIndex; i++)
    {
        const Pour *p = &state.configRecipe.pours[i];
        const int32_t pourWeightMg = (p->ratio / (float)RECIPE_RATIO_MUL) * state.configRecipe.coffeeWeightMg;
        totalPourWeightMg += pourWeightMg;
    }

    const int32_t remainingWeightMg = totalPourWeightMg - (weightSensor.getWeight() * 1000);
    display.recipePour(pour->note, remainingWeightMg, remainingTimePourMs, isPause, recipePourIndex, state.configRecipe.poursCount);
}

void RecipeBrewing::nextPour()
{
    if (recipePourIndex + 1 < state.configRecipe.poursCount)
    {
        recipePourIndex++;
        pourStartMillis = 0;
        pourDoneFlag = false;
    }
}

void RecipeBrewing::enter()
{
    recipePourIndex = 0;
    pourStartMillis = 0;
    pourDoneFlag = false;
}

bool RecipeBrewing::canStepForward() { return recipePourIndex + 1 >= state.configRecipe.poursCount && pourDoneFlag; }
