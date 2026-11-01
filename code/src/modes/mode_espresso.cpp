#include <stdio.h>

#include "modes/mode_espresso.h"
#include "data/localization.h"
#include "formatters.h"
#include "interface.h"
#include "millis.h"
#include "ui/shot_graph.h"
#include "ui/widgets.h"

void ModeEspresso::enter() { Interface::resetEncoderTicks(); }

void ModeEspresso::leave()
{
    if (stopwatch.isRunning())
    {
        stopwatch.stop();
        current.reset();
    }
    showingShot = false;
}

void ModeEspresso::update()
{
    // any tare goes back from the finished shot, e.g. lift-to-tare for the next cup
    if (weightSource.tareCount() != seenTares)
    {
        seenTares = weightSource.tareCount();
        showingShot = false;
    }

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        if (showingShot)
        {
            // only back to the progress screen, the next click starts a shot
            showingShot = false;
        }
        else if (stopwatch.isRunning())
        {
            stopwatch.stop();
            endShot();
        }
        else
        {
            // start stopwatch and tare
            stopwatch.start();
            beginShot();
            weightSource.tare();
            seenTares = weightSource.tareCount();
            approximator.reset();
            startSource = ShotStartSource::Click;
            accepted = false;
            firstDropMs = 0;
            lastReadingMs = now();
        }
    }

    // turning the knob changes the target, which the finished shot doesn't show
    if (Interface::getEncoderTicks() != 0)
    {
        showingShot = false;
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

void ModeEspresso::startShot(unsigned long startMs, ShotStartSource source, uint32_t firstDropMs,
                             const std::vector<Reading> &readings)
{
    stopwatch.startAt(startMs);
    beginShot();
    approximator.reset();
    startSource = source;
    accepted = true;
    this->firstDropMs = firstDropMs;
    lastReadingMs = startMs;
    for (const Reading &reading : readings)
    {
        handleReading(reading);
    }
}

void ModeEspresso::stopShotAt(unsigned long stopMs)
{
    if (stopwatch.isRunning())
    {
        stopwatch.stopAt(stopMs);
        endShot();
    }
}

void ModeEspresso::beginShot()
{
    if (current.isFinished())
    {
        last = current;
    }
    current.reset();
    showingShot = false;
    flow = 0;
}

void ModeEspresso::endShot()
{
    current.finish(stopwatch.getTime());
    showingShot = current.isFinished();
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

    if (showingShot)
    {
        ShotGraph::render(canvas, current, last.isFinished() ? &last : nullptr);
        return;
    }

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
    const int timeBottom = 4 + canvas.ascent();
    Widgets::textHCentered(canvas, buffer, timeBottom);

    // bottom: current and target weight, with a progress bar
    int32_t currentWeightMg = weightSource.getWeight() * 1000;
    float currentWeightG = currentWeightMg / 1000.0;
    float targetWeightG = targetWeightMg / 1000.0;
    canvas.setFont(Font::Number16);
    snprintf(buffer, sizeof(buffer), "%s/%.1fg", formatWeight(currentWeightG, 1), targetWeightG);
    const int weightBaseline = height - BAR_HEIGHT - 8;
    const int weightTop = weightBaseline - canvas.ascent();
    Widgets::textHCentered(canvas, buffer, weightBaseline);

    // between them: the flow, while the shot runs; a lifted cup isn't a negative flow
    if (stopwatch.isRunning())
    {
        canvas.setFont(Font::Small);
        snprintf(buffer, sizeof(buffer), "%.1fg/s", flow > 0 ? flow : 0);
        // the digits centred, the g's tail may reach below
        Widgets::textHCentered(canvas, buffer, (timeBottom + weightTop) / 2 + canvas.ascent() / 2);
    }

    Widgets::progressBar(canvas, 0, height - BAR_HEIGHT, width, BAR_HEIGHT, currentWeightG / targetWeightG);
}

void ModeEspresso::handleReading(const Reading &reading)
{
    // the readings that started a detected shot come again in that loop's update()
    if (!stopwatch.isRunning() || (int32_t)(reading.ms - lastReadingMs) <= 0)
    {
        return;
    }
    lastReadingMs = reading.ms;

    // Readings can wait a little before they arrive, the stopwatch time when each was measured:
    long age = (long)(now() - reading.ms);
    long time = (long)stopwatch.getTime() - age;
    if (time <= 0)
    {
        // Measured before the stopwatch started, or in the same millisecond:
        // the readings of the loop that starts it were made before its tare.
        return;
    }

    current.add(time, reading.net);
    flow = reading.flow;

    int32_t weightMg = reading.net * 1000;
    approximator.addPoint({time, (float)weightMg});
    lastEstimatedTime = approximator.getXAtY(targetWeightMg);

    if (!accepted && weightMg >= targetWeightMg)
    {
        stopwatch.stop();
        endShot();
    }
}

bool ModeEspresso::canSwitchMode() { return true; }

const char *ModeEspresso::getName() { return MODE_NAME_ESPRESSO; }
