#include "modes/mode_settings.h"
#include "user_interface.h"
#include "data/localization.h"

void ModeSettings::update()
{
    return;
}

bool ModeSettings::canSwitchMode()
{
    return true;
}

const char* ModeSettings::getName()
{
    return MODE_NAME_SETTINGS;
}