#pragma once

#include "detectors/auto_tare.h"
#include "mode.h"
#include "stopwatch.h"
#include "weighing/weight_source.h"
#include "settings.h"

#include <vector>

/** An entry at the end of the settings menu that runs something when clicked, e.g. the Wi-Fi setup. */
struct SettingsAction
{
    const char *name;
    /** Runs to its end, drawing its own screens. */
    void (*run)();
};

class ModeSettings : public Mode
{
public:
    /**
     * @param onLeave runs when the mode is left, so changed settings apply without a restart
     * @param grinderIp writes the scale's IP address into out, or nothing if it has none, shown
     *                  under the Mahlkönig grinder setting when it is On
     */
    explicit ModeSettings(const SettingsAction *actions = nullptr, int actionCount = 0, void (*onLeave)() = nullptr,
                          void (*grinderIp)(char *out, size_t size) = nullptr);
    void update() override;
    void render(Canvas &canvas) override;
    void enter() {
        selected = 0;
        modifySetting = false;
        // applied at startup and on every leave, so what is stored now is what is active
        grinderActive = Settings::mahlkoenig();
    };
    void leave() override
    {
        if (onLeave != nullptr)
        {
            onLeave();
        }
    }
    bool canSwitchMode();
    const char* getName();
private:
    void updateSwitcher();
    void updateFloatSetting();
    void updateChoiceSetting();
    float editedValue();
    int editedChoice();
    /** The choice settings come after the float settings in the menu, the actions after them. */
    bool isChoice() const { return selected >= Settings::FLOAT_SETTING_NUM && selected < Settings::SETTING_COUNT; }
    bool isAction() const { return selected >= Settings::SETTING_COUNT; }
    int entryCount() const { return Settings::SETTING_COUNT + actionCount; }
    const Settings::ChoiceSetting &selectedChoice() const
    {
        return Settings::choiceSettings[selected - Settings::FLOAT_SETTING_NUM];
    }
    int selected = 0;
    bool modifySetting = false;
    /** Whether the Mahlkönig grinder setting was on when the settings were entered. */
    bool grinderActive = false;
    // value of the setting being modified, saved on click
    float value = 0;
    // level of the choice setting being modified, saved on click
    int choice = 0;
    const SettingsAction *actions;
    int actionCount;
    void (*onLeave)();
    void (*grinderIp)(char *out, size_t size);
    /** The names of the settings, then of the actions. */
    std::vector<const char *> options;
};
