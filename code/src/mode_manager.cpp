#include <math.h>
#include <stdio.h>

#include "mode_manager.h"
#include "millis.h"
#include "battery.h"
#include "interface.h"
#include "ui/widgets.h"

ModeManager::ModeManager(Mode *modes[], const int modeCount, Canvas &canvas)
    : modes(modes), modeCount(modeCount), currentMode(0), inModeChange(false), lastVoltage(0), lastPercentage(0),
      lastCharging(false), lastBatteryTime(0), canvas(canvas),
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
    unsigned long time = now();
    if (!renderNow && time - lastRenderTime < RENDER_INTERVAL_MS)
    {
        return;
    }

    lastRenderTime = time;
    renderNow = false;

    canvas.clear();
    if (inModeChange)
    {
        renderModeSwitcher();
    }
    else
    {
        modes[currentMode]->render(canvas);
    }
    canvas.flush();
}

void ModeManager::renderModeSwitcher()
{
    static const int PADDING = 5;

    Widgets::segmentBar(canvas, currentMode, modeCount);

    canvas.setFont(Font::Mono20);
    Widgets::textCentered(canvas, modes[currentMode]->getName());

    // battery icon, rotated to stand upright in the bottom left corner
    canvas.setFont(Font::Battery);
    uint16_t glyph = '0' + (lastCharging ? 6 : (uint16_t)roundf(lastPercentage / 20));
    canvas.drawGlyph(PADDING, canvas.height() - 8 - PADDING, glyph, 1);

    if (!lastCharging)
    {
        canvas.setFont(Font::Body);
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2fV", lastVoltage);
        canvas.drawText(canvas.width() - canvas.textWidth(buffer) - PADDING, canvas.height() - PADDING, buffer);
    }
}

void ModeManager::updateModeChange()
{
    int previousMode = currentMode;
    currentMode += static_cast<int>(Interface::getEncoderDirection());

    if (currentMode < 0)
    {
        currentMode = 0;
    }
    else if (currentMode >= modeCount)
    {
        currentMode = modeCount - 1;
    }

    if (currentMode != previousMode)
    {
        renderNow = true;
    }

    if (now() > lastBatteryTime + BATTERY_UPDATE_INTERVAL || lastBatteryTime == 0)
    {
        lastBatteryTime = now();
        lastVoltage = Battery::getVoltage();
        lastPercentage = Battery::getPercentage();
    }
    // cheap to read, so plugging in shows right away
    lastCharging = Battery::isCharging();

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        inModeChange = false;
        modes[currentMode]->enter();
        // show the new mode right away instead of waiting for the next frame slot
        renderNow = true;
    }
}

void ModeManager::update()
{
    if (inModeChange)
    {
        updateModeChange();
        if (!inModeChange)
        {
            // the selected mode renders after its first update()
            return;
        }
    }
    else if (Interface::getEncoderClick() == ClickType::LONG && modes[currentMode]->canSwitchMode())
    {
        // the switcher is shown from the next update() on
        inModeChange = true;
        renderNow = true;
        return;
    }
    else
    {
        modes[currentMode]->update();
    }

    renderIfDue();
}
