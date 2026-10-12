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
    if (!modifySetting)
    {
        updateSwitcher();
    }
    else if (isChoice())
    {
        updateChoiceSetting();
    }
    else
    {
        updateFloatSetting();
    }
}

bool ModeSettings::canSwitchMode() { return !modifySetting; }

const char *ModeSettings::getName() { return MODE_NAME_SETTINGS; }

void ModeSettings::updateSwitcher()
{
    selected += Interface::getEncoderTurns();
    if (selected < 0)
    {
        selected = 0;
    }
    else if (selected >= Settings::SETTING_COUNT)
    {
        selected = Settings::SETTING_COUNT - 1;
    }

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        modifySetting = true;
        Interface::resetEncoderTicks();
        // render() shows the value already in this tick
        if (isChoice())
        {
            choice = editedChoice();
        }
        else
        {
            value = editedValue();
        }
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

/** The stored level turned by the encoder, kept within the levels. */
int ModeSettings::editedChoice()
{
    const Settings::ChoiceSetting &setting = selectedChoice();
    const int stored = Settings::getChoice(setting);
    int edited = stored + (int)Interface::getEncoderTicks();
    if (edited < 0 || edited >= setting.count)
    {
        edited = edited < 0 ? 0 : setting.count - 1;
        // turning back leaves the end right away
        Interface::setEncoderTicks(edited - stored);
    }
    return edited;
}

void ModeSettings::updateChoiceSetting()
{
    choice = editedChoice();

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        modifySetting = false;
        Settings::setChoice(selectedChoice(), choice);
    }
    else if (Interface::getEncoderClick() == ClickType::LONG)
    {
        modifySetting = false;
    }
}

/** As many decimals as a setting's step needs, at least one, e.g. 2 for 0.05. */
static int decimalsFor(float increment)
{
    int decimals = 1;
    while (decimals < 3)
    {
        float scaled = increment * powf(10, decimals);
        if (fabsf(scaled - roundf(scaled)) < 0.001f)
        {
            break;
        }
        decimals++;
    }
    return decimals;
}

void ModeSettings::render(Canvas &canvas)
{
    if (modifySetting && isChoice())
    {
        canvas.setFont(Font::Medium);
        Widgets::textHCentered(canvas, selectedChoice().labels[choice], 40);
    }
    else if (modifySetting)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.*f", decimalsFor(Settings::floatSettings[selected].increment), value);
        canvas.setFont(Font::Number16);
        Widgets::textHCentered(canvas, buffer, 40);
    }
    else
    {
        Widgets::switcher(canvas, MODE_NAME_SETTINGS, selected, Settings::SETTING_COUNT, Settings::getOptions());
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
    }
    else if (Interface::getEncoderClick() == ClickType::LONG)
    {
        modifySetting = false;
    }
}
