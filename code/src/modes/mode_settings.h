#pragma once

#include "auto_tare.h"
#include "mode.h"
#include "stopwatch.h"
#include "weight_sensor.h"
#include "settings.h"

class ModeSettings : public Mode
{
public:
    void update();
    void enter() {
        selected = 0;
        modifySetting = false;
    };
    bool canSwitchMode();
    const char* getName();
private:
    void updateSwitcher();
    void updateFloatSetting();
    int selected = 0;
    bool modifySetting = false;
};