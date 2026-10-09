#include <math.h>

#include "modes/mode_scale.h"
#include "data/localization.h"
#include "formatters.h"
#include "interface.h"
#include "logger.h"
#include "settings.h"
#include "weight_sensor.h"

void ModeScale::update()
{
    if (Interface::getEncoderDirection() != Interface::EncoderDirection::NONE)
    {
        weightSensor.tare();
    }

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();
    }

    if (weightSensor.isNewWeight())
    {
        autoTare->update(weightSensor.getLastUntaredWeight());
        if (autoTare->shouldTare()) {
            Interface::buzzerTone(100);
            weightSensor.tare();
        }
    }
}

void ModeScale::render(Canvas &canvas)
{
    canvas.setFont(Font::Number30);
    canvas.drawText(0, 30, formatWeight(weightSensor.getWeight()));
    canvas.setFont(Font::Number22);
    canvas.drawText(0, 64, formatTime(stopwatch.getTime()));
}

void ModeScale::enter() {
    // set correct values for auto tare
    autoTare->weights = Settings::getAllAutoTares();
    for (auto weight : autoTare->weights)
    {
        LOGI("Scale", "auto tare weights: %f\n", weight);
    }

    float tolerance = Settings::getFloat(Settings::floatSettings[Settings::AUTO_TARE_TOLERANCE]);
    if (!isnan(tolerance))
    {
        autoTare->tolerance = tolerance;
    }

    LOGI("Scale", "auto tare tolerance: %f\n", autoTare->tolerance);
}

bool ModeScale::canSwitchMode()
{
    return true;
}

const char* ModeScale::getName()
{
    return MODE_NAME_SCALE;
}