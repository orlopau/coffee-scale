#include "modes/mode_scale.h"
#include "scale.h"
#include "data/localization.h"
#include "interface.h"

void ModeScale::update()
{
    display.display(weightSensor.getWeight(), stopwatch.getTime());

    if (Interface::getEncoderDirection() != Interface::EncoderDirection::NONE)
    {
        weightSensor.tare();
    }

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();
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