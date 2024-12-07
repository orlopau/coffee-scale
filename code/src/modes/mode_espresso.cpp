#include "modes/mode_espresso.h"
#include "data/localization.h"
#include "interface.h"

void ModeEspresso::enter() { Interface::resetEncoderTicks(); }

void ModeEspresso::update()
{
    // start stopwatch and tare on click
    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();

        if (stopwatch.isRunning())
        {
            weightSensor.tare();
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

    // new weight must be handled for regression
    if (weightSensor.isNewWeight())
    {
        handleNewWeight();
    }

    // display
    int32_t remainingTime = lastEstimatedTime - stopwatch.getTime();
    bool waiting =
        !stopwatch.isRunning() || remainingTime < 0 || remainingTime > REGRESSION_MAX_TIME || stopwatch.getTime() < REGRESSION_GRACE_PERIOD;

    display.espressoShot(stopwatch.getTime(), remainingTime, weightSensor.getWeight() * 1000, targetWeightMg, waiting);
}

void ModeEspresso::handleNewWeight()
{
    if (stopwatch.isRunning())
    {
        int32_t lastWeightMg = weightSensor.getLastWeight() * 1000;
        approximator.addPoint({(long)stopwatch.getTime(), (float)lastWeightMg});
        lastEstimatedTime = approximator.getXAtY(targetWeightMg);

        if (lastWeightMg >= targetWeightMg)
        {
            stopwatch.stop();
        }
    }
}

bool ModeEspresso::canSwitchMode() { return true; }

const char *ModeEspresso::getName() { return MODE_NAME_ESPRESSO; }