#include <stdio.h>

#include "step_brewing.h"
#include "millis.h"
#include "interface.h"
#include "ui/widgets.h"

RecipeBrewing::RecipeBrewing(RecipeStepState &state, WeightSensor &weightSensor)
    : state(state), weightSensor(weightSensor)
{
}

void RecipeBrewing::update()
{
    const Pour *pour = &state.configRecipe.pours[recipePourIndex];

    uint64_t remainingTimePourMs;
    bool isPause = false;

    // tare scale on rotation
    if (Interface::getEncoderDirection() != Interface::EncoderDirection::NONE)
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
        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            pourStartMillis = now();
            // consume, else we will advance to next pour in a later if
            Interface::consumeEncoderClick();
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
            Interface::buzzerTone(200);
        }

        // if there is another pour, start when auto advance is on
        if (pour->autoAdvance)
        {
            nextPour();
        }
    }

    // if brew has started, clicking should advance to next pour
    if (pourStartMillis != 0 && Interface::getEncoderClick() == ClickType::SINGLE)
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

    remainingWeightMg = totalPourWeightMg - (weightSensor.getWeight() * 1000);
    remainingTimeMs = remainingTimePourMs;
    pausing = isPause;
}

void RecipeBrewing::render(Canvas &canvas)
{
    static const int TEXT_X_PADDING = 3;

    const int width = canvas.width();
    char buffer[16];

    // top: one segment per pour with the current one filled, then the pour's note
    canvas.setFont(Font::Body);
    const int bodyAscent = canvas.ascent();
    int y = Widgets::segmentBar(canvas, recipePourIndex, state.configRecipe.poursCount);
    y += 2;
    Widgets::textWrapped(canvas, state.configRecipe.pours[recipePourIndex].note, y, 0, width);

    // bottom: weight to pour and remaining time, below a line
    canvas.setFont(Font::Mono13);
    y = canvas.height() - (canvas.ascent() - canvas.descent()) - 6;
    canvas.drawHLine(0, y, width);
    y += 3;
    const int center = y + (canvas.height() - y) / 2.0;

    snprintf(buffer, sizeof(buffer), "%.2fg", -1 * remainingWeightMg / 1000.0);
    canvas.drawText(TEXT_X_PADDING, center + bodyAscent / 2.0, buffer);

    snprintf(buffer, sizeof(buffer), pausing ? "TP-%02d:%02d" : "T-%02d:%02d", (int)(remainingTimeMs / 1000 / 60),
             (int)(remainingTimeMs / 1000 % 60));
    canvas.drawText(width - canvas.textWidth(buffer) - TEXT_X_PADDING, center + bodyAscent / 2.0, buffer);
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
