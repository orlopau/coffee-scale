#include <math.h>
#include <stdio.h>

#include "mode_settings.h"
#include "data/localization.h"
#include "ui/widgets.h"
#include "interface.h"
#include "logger.h"
#include "settings.h"

ModeSettings::ModeSettings(const SettingsAction *actions, int actionCount, void (*onLeave)(),
                           void (*grinderIp)(char *out, size_t size))
    : actions(actions), actionCount(actionCount), onLeave(onLeave), grinderIp(grinderIp)
{
    const char **settings = Settings::getOptions();
    options.assign(settings, settings + Settings::SETTING_COUNT);
    for (int i = 0; i < actionCount; i++)
    {
        options.push_back(actions[i].name);
    }
}

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
    else if (selected >= entryCount())
    {
        selected = entryCount() - 1;
    }

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        if (isAction())
        {
            actions[selected - Settings::SETTING_COUNT].run();
            // the action took the clicks it wanted, the rest were meant for it
            Interface::discardClicks();
            return;
        }
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
        if (selectedChoice().id == Settings::MAHLKOENIG && choice == 1)
        {
            // the grinder is told this name or address, so it is shown under the label
            Widgets::textHCentered(canvas, selectedChoice().labels[choice], 28);
            char ip[16] = "";
            if (grinderIp != nullptr)
            {
                grinderIp(ip, sizeof(ip));
            }
            canvas.setFont(Font::Body);
            Widgets::textHCentered(canvas, Settings::GRINDER_HOST, 44);
            // turned on just now, nothing has tried to connect yet
            Widgets::textHCentered(canvas,
                                   !grinderActive  ? GRINDER_ACTIVE_AFTER_LEAVING
                                   : ip[0] != '\0' ? ip
                                                   : GRINDER_NOT_CONNECTED,
                                   58);
        }
        else
        {
            Widgets::textHCentered(canvas, selectedChoice().labels[choice], 40);
        }
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
        Widgets::switcher(canvas, MODE_NAME_SETTINGS, selected, entryCount(), options.data());
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
