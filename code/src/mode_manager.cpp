#include "mode_manager.h"
#include "millis.h"

ModeManager::ModeManager(Mode *modes[], const int modeCount, Display &display, UserInput &input)
    : modes(modes), modeCount(modeCount), currentMode(0), inModeChange(false), display(display), input(input) {}

void ModeManager::update()
{
    if (inModeChange)
    {
        currentMode += static_cast<int>(input.getEncoderDirection());

        if (currentMode < 0)
        {
            currentMode = 0;
        }
        else if (currentMode >= modeCount)
        {
            currentMode = modeCount - 1;
        }

        display.centerText(modes[currentMode]->getName(), 16);

        if (input.getEncoderClick() == ClickType::SINGLE)
        {
            inModeChange = false;
        }
    }
    else
    {
        if (input.getEncoderClick() == ClickType::LONG && modes[currentMode]->canSwitchMode())
        {
            inModeChange = true;
        }
        else
        {
            modes[currentMode]->update();
        }
    }
}