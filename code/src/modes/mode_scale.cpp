#include "modes/mode_scale.h"
#include "weight_sensor.h"
#include "data/localization.h"
#include "interface.h"
#include "display.h"

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
            weightSensor.tare();
        }
    }
}

bool ModeScale::canSwitchMode()
{
    return true;
}

const char* ModeScale::getName()
{
    return MODE_NAME_SCALE;
}