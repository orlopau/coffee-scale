#include "mode_manager.h"
#include "millis.h"

ModeManager::ModeManager(Mode *modes[], const char *modeNames[], const int modeCount, Display &display)
    : modes(modes), modeCount(modeCount), currentMode(0), inModeChange(false), modeNames(modeNames),
      lastChangeMillis(0), display(display) {}

void ModeManager::update(const int modeChange)
{
    if (modeChange != 0)
    {
        currentMode += modeChange;
        inModeChange = true;
        lastChangeMillis = now();

        if (currentMode < 0)
        {
            currentMode = 0;
        }
        else if (currentMode >= modeCount)
        {
            currentMode = modeCount - 1;
        }
    }

    if (inModeChange)
    {
        if (now() - lastChangeMillis > MODE_SWITCH_DELAY_MS)
        {
            inModeChange = false;
        }
    }

    if (!inModeChange)
    {
        modes[currentMode]->update();
    }
    else
    {
        display.centerText(modeNames[currentMode], 30);
    }
}