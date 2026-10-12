#pragma once

#include "detectors/auto_tare.h"
#include "mode.h"
#include "stopwatch.h"
#include "weighing/weight_source.h"
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
    void updateChoiceSetting();
    float editedValue();
    int editedChoice();
    /** The choice settings come after the float settings in the menu. */
    bool isChoice() const { return selected >= Settings::FLOAT_SETTING_NUM; }
    const Settings::ChoiceSetting &selectedChoice() const
    {
        return Settings::choiceSettings[selected - Settings::FLOAT_SETTING_NUM];
    }
    int selected = 0;
    bool modifySetting = false;
    // value of the setting being modified, saved on click
    float value = 0;
    // level of the choice setting being modified, saved on click
    int choice = 0;
};
