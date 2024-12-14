#include "modes/mode_scale.h"
#include "weight_sensor.h"
#include "data/localization.h"
#include "interface.h"
#include "display.h"
#include "settings.h"

void ModeScale::update()
{
    Display::display(weightSensor.getWeight(), stopwatch.getTime());

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

void ModeScale::enter() {
    // set correct values for auto tare
    autoTare->weights = Settings::getAllAutoTares();
}

bool ModeScale::canSwitchMode()
{
    return true;
}

const char* ModeScale::getName()
{
    return MODE_NAME_SCALE;
}