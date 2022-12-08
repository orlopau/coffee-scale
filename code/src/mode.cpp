#include "mode.h"
#include "loadcell.h"
#include "user_interface.h"
#include "formatters.h"
#include "millis.h"

void ModeDefault::update()
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

bool ModeDefault::canSwitchMode()
{
    return true;
}

const char* ModeDefault::getName()
{
    return "Scale";
}