#include <stdio.h>

#include "modes/mode_espresso.h"
#include "data/localization.h"
#include "formatters.h"
#include "interface.h"
#include "millis.h"
#include "ui/widgets.h"

void ModeEspresso::enter() { Interface::resetEncoderTicks(); }

void ModeEspresso::update()
{
    // start stopwatch and tare on click
    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();

        if (stopwatch.isRunning())
        {
            weightSource.tare();
            approximator.reset();
        }
    }

    // clamp target weight to min and max
    targetWeightMg += (Interface::getEncoderTicks() * ENCODER_MG_PER_TICK);
    if (targetWeightMg < MIN_TARGET_WEIGHT_MG)
    {
        targetWeightMg = MIN_TARGET_WEIGHT_MG;
    }
    else if (targetWeightMg > MAX_TARGET_WEIGHT_MG)
    {
        targetWeightMg = MAX_TARGET_WEIGHT_MG;
    }

    if (Interface::getEncoderTicks() >= 1 || Interface::getEncoderTicks() <= -1)
    {
        Interface::resetEncoderTicks();
    }

    // every reading goes into the regression
    for (const Reading &reading : weightSource.newReadings())
    {
        handleReading(reading);
    }
}

int32_t ModeEspresso::getRemainingTimeMs() { return lastEstimatedTime - stopwatch.getTime(); }

bool ModeEspresso::isWaitingForEstimate()
{
    int32_t remainingTime = getRemainingTimeMs();
    return !stopwatch.isRunning() || remainingTime < 0 || remainingTime > REGRESSION_MAX_TIME ||
           stopwatch.getTime() < REGRESSION_GRACE_PERIOD;
}

void ModeEspresso::render(Canvas &canvas)
{
    static const int BAR_HEIGHT = 5;

    const int width = canvas.width();
    const int height = canvas.height();
    char buffer[32];

    // top: elapsed time, and remaining time once there is an estimate
    float currentTimeS = (uint32_t)stopwatch.getTime() / 1000.0;
    canvas.setFont(Font::Number18);
    if (isWaitingForEstimate())
    {
        snprintf(buffer, sizeof(buffer), "%.1fs", currentTimeS);
    }
    else
    {
        float remainingTimeS = (uint32_t)getRemainingTimeMs() / 1000.0;
        snprintf(buffer, sizeof(buffer), "%.1fs|%.1fs", -remainingTimeS, currentTimeS);
    }
    Widgets::textHCentered(canvas, buffer, 4 + canvas.ascent());

    // bottom: current and target weight, with a progress bar
    int32_t currentWeightMg = weightSource.getWeight() * 1000;
    float currentWeightG = currentWeightMg / 1000.0;
    float targetWeightG = targetWeightMg / 1000.0;
    canvas.setFont(Font::Number16);
    snprintf(buffer, sizeof(buffer), "%s/%.1fg", formatWeight(currentWeightG, 1), targetWeightG);
    Widgets::textHCentered(canvas, buffer, height - BAR_HEIGHT - 8);

    Widgets::progressBar(canvas, 0, height - BAR_HEIGHT, width, BAR_HEIGHT, currentWeightG / targetWeightG);
}

void ModeEspresso::handleReading(const Reading &reading)
{
    if (!stopwatch.isRunning())
    {
        return;
    }

    // Readings can wait a little before they arrive, the stopwatch time when each was measured:
    long age = (long)(now() - reading.ms);
    long time = (long)stopwatch.getTime() - age;
    if (time <= 0)
    {
        // Measured before the stopwatch started, or in the same millisecond:
        // the readings of the loop that starts it were made before its tare.
        return;
    }

    int32_t weightMg = reading.net * 1000;
    approximator.addPoint({time, (float)weightMg});
    lastEstimatedTime = approximator.getXAtY(targetWeightMg);

    if (weightMg >= targetWeightMg)
    {
        stopwatch.stop();
    }
}

bool ModeEspresso::canSwitchMode() { return true; }

const char *ModeEspresso::getName() { return MODE_NAME_ESPRESSO; }
