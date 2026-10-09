#pragma once

#include "auto_tare.h"
#include "mode.h"
#include "stopwatch.h"
#include "weight_sensor.h"
#include "settings.h"

class ModeSettings : public Mode
{
public:
    void update() override;
    void render(Canvas &canvas) override;
    void enter() {
        selected = 0;
        modifySetting = false;
    };
    bool canSwitchMode();
    const char* getName();
private:
    void updateSwitcher();
    void updateFloatSetting();
    float editedValue();
    int selected = 0;
    bool modifySetting = false;
    // value of the setting being modified, saved on click
    float value = 0;
};