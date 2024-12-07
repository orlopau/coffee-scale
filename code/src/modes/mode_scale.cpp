#include "modes/mode_scale.h"
#include "scale.h"
#include "user_interface.h"
#include "data/localization.h"

void ModeScale::update()
{
    display.display(weightSensor.getWeight(), stopwatch.getTime());

    if (buttons.getEncoderDirection() != EncoderDirection::NONE)
    {
        weightSensor.tare();
    }

    if (buttons.getEncoderClick() == ClickType::SINGLE)
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