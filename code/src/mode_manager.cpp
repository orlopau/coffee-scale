#include "mode_manager.h"
#include "util/millis.h"

ModeManager::ModeManager(Mode *modes[], const int modeCount, Display &display, UserInput &input, Battery &battery)
    : modes(modes), modeCount(modeCount), currentMode(0), inModeChange(false), display(display), input(input), battery(battery),
      lastBatteryTime(0)
{
}

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

        if (now() > lastBatteryTime + BATTERY_UPDATE_INTERVAL || lastBatteryTime == 0)
        {
            lastBatteryTime = now();
            lastVoltage = battery.getVoltage();
            lastPercentage = battery.getPercentage();
        }

        display.modeSwitcher(modes[currentMode]->getName(), currentMode, modeCount, lastVoltage, lastPercentage, battery.isCharging());

        if (input.getEncoderClick() == ClickType::SINGLE)
        {
            inModeChange = false;
            modes[currentMode]->enter();
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