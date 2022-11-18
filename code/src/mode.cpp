#include "mode.h"
#include "loadcell.h"
#include "user_interface.h"

void ModeDefault::update()
{
    display.display(loadCell.getWeight(), stopwatch.getTime());

    if (buttons.getBootClick() == ClickType::SINGLE)
    {
        loadCell.tare();
    }

    if (buttons.getEncoderClick() == ClickType::SINGLE)
    {
        stopwatch.toggle();
    }
}