#include "mode_manager.h"
#include "millis.h"
#include "battery.h"
#include "interface.h"
#include "display.h"

ModeManager::ModeManager(Mode *modes[], const int modeCount, Canvas *canvas)
    : modes(modes), modeCount(modeCount), currentMode(0), inModeChange(false), lastBatteryTime(0), canvas(canvas),
      lastRenderTime(0), renderNow(true)
{
}

void ModeManager::begin()
{
    modes[currentMode]->enter();
    renderNow = true;
}

void ModeManager::renderIfDue()
{
    Mode *mode = modes[currentMode];
    if (canvas == nullptr || !mode->rendersToCanvas())
    {
        return;
    }

    unsigned long time = now();
    if (!renderNow && time - lastRenderTime < RENDER_INTERVAL_MS)
    {
        return;
    }

    lastRenderTime = time;
    renderNow = false;

    canvas->clear();
    mode->render(*canvas);
    canvas->flush();
}

void ModeManager::update()
{
    if (inModeChange)
    {
        currentMode += static_cast<int>(Interface::getEncoderDirection());

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
            lastVoltage = Battery::getVoltage();
            lastPercentage = Battery::getPercentage();
        }

        Display::modeSwitcher(modes[currentMode]->getName(), currentMode, modeCount, lastVoltage, lastPercentage, Battery::isCharging());

        if (Interface::getEncoderClick() == ClickType::SINGLE)
        {
            inModeChange = false;
            modes[currentMode]->enter();
            // show the new mode right away instead of waiting for the next frame slot
            renderNow = true;
        }
    }
    else
    {
        if (Interface::getEncoderClick() == ClickType::LONG && modes[currentMode]->canSwitchMode())
        {
            inModeChange = true;
        }
        else
        {
            modes[currentMode]->update();
            renderIfDue();
        }
    }
}