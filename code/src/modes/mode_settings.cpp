#include "mode_settings.h"
#include "data/localization.h"
#include "display.h"
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
    }

    Display::switcher(MODE_NAME_SETTINGS, selected, Settings::FLOAT_SETTING_NUM, Settings::floatSettingNames);
}

void ModeSettings::updateFloatSetting()
{
    float val = Settings::getFloat(static_cast<Settings::FloatSetting>(selected));
    if (isnan(val)) {
        val = 0;
    }

    val += (float) Interface::getEncoderTicks() * 0.5f;

    // display value
    static char buffer[32];
    sprintf(buffer, "%.1f", val);
    Display::centerText(buffer, 16);

    if (Interface::getEncoderClick() == ClickType::SINGLE)
    {
        modifySetting = false;
        Settings::setFloat(static_cast<Settings::FloatSetting>(selected), val);
        Settings::commit();
    }
    else if (Interface::getEncoderClick() == ClickType::LONG)
    {
        modifySetting = false;
    }
}
