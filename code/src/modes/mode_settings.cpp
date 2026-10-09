#include <math.h>
#include <stdio.h>

#include "mode_settings.h"
#include "data/localization.h"
#include "ui/widgets.h"
#include "interface.h"
#include "logger.h"
#include "settings.h"

void ModeSettings::update()
{
    if (modifySetting)
    {
        updateFloatSetting();
    }
    else
    {
        updateSwitcher();
    }
}

bool ModeSettings::canSwitchMode() { return !modifySetting; }

const char *ModeSettings::getName() { return MODE_NAME_SETTINGS; }

void ModeSettings::updateSwitcher()
{
    selected += static_cast<int>(Interface::getEncoderDirection());
    if (selected < 0)
    {
        selected = 0;
    }
    else if (selected >= Settings::FLOAT_SETTING_NUM)
    {
        selected = Settings::FLOAT_SETTING_NUM - 1;
    }

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        modifySetting = true;
        Interface::resetEncoderTicks();
        // render() shows the value already in this tick
        value = editedValue();
    }
}

float ModeSettings::editedValue()
{
    auto setting = Settings::floatSettings[selected];
    float stored = Settings::getFloat(setting);
    if (isnan(stored))
    {
        stored = 0;
    }
    return stored + (float)Interface::getEncoderTicks() * setting.increment;
}

void ModeSettings::render(Canvas &canvas)
{
    if (modifySetting)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.1f", value);
        canvas.setFont(Font::Number16);
        Widgets::textHCentered(canvas, buffer, 40);
    }
    else
    {
        Widgets::switcher(canvas, MODE_NAME_SETTINGS, selected, Settings::FLOAT_SETTING_NUM, Settings::getOptions());
    }
}

void ModeSettings::updateFloatSetting()
{
    auto setting = Settings::floatSettings[selected];
    value = editedValue();

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        modifySetting = false;
        Settings::setFloat(setting, value);
        Settings::commit();
    }
    else if (Interface::getEncoderClick() == ClickType::LONG)
    {
        modifySetting = false;
    }
}
