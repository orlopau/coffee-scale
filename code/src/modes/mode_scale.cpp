#include <math.h>

#include "modes/mode_scale.h"
#include "data/localization.h"
#include "formatters.h"
#include "interface.h"
#include "logger.h"
#include "settings.h"
#include "weighing/weight_source.h"

void ModeScale::update()
{
    if (Interface::getEncoderDirection() != Interface::EncoderDirection::NONE)
    {
        weightSource.tare();
    }

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();
    }

    for (const Reading &reading : weightSource.newReadings())
    {
        autoTare->update(reading.grams);
        if (autoTare->shouldTare()) {
            Interface::buzzerTone(100);
            weightSource.tare();
        }
    }
}

void ModeScale::render(Canvas &canvas)
{
    canvas.setFont(Font::Number30);
    canvas.drawText(0, 30, formatWeight(weightSource.getWeight()));
    canvas.setFont(Font::Number22);
    canvas.drawText(0, 64, formatTime(stopwatch.getTime()));
}

void ModeScale::enter() {
    // set correct values for auto tare
    autoTare->weights = Settings::getAllAutoTares();
    for (auto weight : autoTare->weights)
    {
        LOGI("Scale", "auto tare weights: %f", weight);
    }

    float tolerance = Settings::getFloat(Settings::floatSettings[Settings::AUTO_TARE_TOLERANCE]);
    if (!isnan(tolerance))
    {
        autoTare->tolerance = tolerance;
    }

    LOGI("Scale", "auto tare tolerance: %f", autoTare->tolerance);
}

bool ModeScale::canSwitchMode()
{
    return true;
}

const char* ModeScale::getName()
{
    return MODE_NAME_SCALE;
}