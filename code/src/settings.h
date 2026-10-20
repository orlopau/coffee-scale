#pragma once

#include "data/localization.h"
#include <vector>
#include <cmath>
#include <array>
#include <stddef.h>
#include <stdint.h>

namespace Settings
{
    struct FloatSetting
    {
        int id;
        /** NVS key, at most 15 characters. Never rename it, the scales have their values stored under it. */
        const char* key;
        const char* name;
        float def;
        float increment;
    };

    enum FloatSettingID
    {
        AUTO_TARE_0 = 0,
        AUTO_TARE_1,
        AUTO_TARE_2,
        AUTO_TARE_3,
        AUTO_TARE_4,
        AUTO_TARE_TOLERANCE,
        AVERAGING_DELTA,
        ZERO_TRACKING,
        SHOT_OFFSET,
        SHOT_TIME,
        FLOAT_SETTING_NUM
    };

    constexpr std::array<FloatSetting, FLOAT_SETTING_NUM> floatSettings = {{
        {AUTO_TARE_0, "tare0", SETTINGS_TARE_0, NAN, 0.5f},
        {AUTO_TARE_1, "tare1", SETTINGS_TARE_1, NAN, 0.5f},
        {AUTO_TARE_2, "tare2", SETTINGS_TARE_2, NAN, 0.5f},
        {AUTO_TARE_3, "tare3", SETTINGS_TARE_3, NAN, 0.5f},
        {AUTO_TARE_4, "tare4", SETTINGS_TARE_4, NAN, 0.5f},
        {AUTO_TARE_TOLERANCE, "tareTol", SETTINGS_TARE_TOLERANCE, NAN, 0.1f},
        {AVERAGING_DELTA, "avgDelta", SETTINGS_AVERAGING_DELTA, 1.0f, 0.1f},
        // the window in grams around zero, 0 turns zero tracking off
        {ZERO_TRACKING, "zeroTrack", SETTINGS_ZERO_TRACKING, 0.1f, 0.05f},
        // seconds from the pump start to the first drop, a detected shot starts that much earlier
        {SHOT_OFFSET, "shotOffset", SETTINGS_SHOT_OFFSET, 0.0f, 0.5f},
        // seconds a shot usually takes, with the target weight the flow the shot detection expects
        {SHOT_TIME, "shotTime", SETTINGS_SHOT_TIME, 30.0f, 1.0f},
    }};

    /** A setting with a few named levels, stored as the level's index. */
    struct ChoiceSetting
    {
        int id;
        /** NVS key, at most 15 characters. Never rename it, the scales have their values stored under it. */
        const char *key;
        const char *name;
        /** Index of the default level. */
        uint8_t def;
        uint8_t count;
        /** count labels, one per level. */
        const char *const *labels;
    };

    enum ChoiceSettingID
    {
        LIFT_TARE = 0,
        SHOT_AUTO,
        CHOICE_SETTING_NUM
    };

    constexpr const char *LIFT_TARE_LABELS[] = {
        SETTINGS_LIFT_TARE_OFF,
        SETTINGS_LIFT_TARE_GENTLE,
        SETTINGS_LIFT_TARE_NORMAL,
        SETTINGS_LIFT_TARE_FIRM,
    };
    /**
     * How deep a lift has to dip per level, in grams, 0 is off. The button's
     * spring-back reached 0.37 g, the shallowest recorded lift 0.94 g, see
     * docs/adr/0001-tare-gesture.md.
     */
    constexpr float LIFT_TARE_DEPTHS[] = {0, 0.5f, 0.7f, 1.5f};
    static_assert(sizeof(LIFT_TARE_DEPTHS) / sizeof(LIFT_TARE_DEPTHS[0]) ==
                      sizeof(LIFT_TARE_LABELS) / sizeof(LIFT_TARE_LABELS[0]),
                  "one depth per label");

    /** Whether a shot detected in the scale mode switches to espresso. Off leaves detection on the espresso screen on. */
    constexpr const char *SHOT_AUTO_LABELS[] = {
        SETTINGS_SHOT_AUTO_OFF,
        SETTINGS_SHOT_AUTO_ON,
    };

    constexpr std::array<ChoiceSetting, CHOICE_SETTING_NUM> choiceSettings = {{
        {LIFT_TARE, "liftTare", SETTINGS_LIFT_TARE, 2, 4, LIFT_TARE_LABELS},
        {SHOT_AUTO, "shotAuto", SETTINGS_SHOT_AUTO, 1, 2, SHOT_AUTO_LABELS},
    }};

    /** Entries of the settings menu: the float settings, then the choice settings. */
    constexpr int SETTING_COUNT = FLOAT_SETTING_NUM + CHOICE_SETTING_NUM;

    /** NVS key of the calibration, in grams per count. */
    constexpr const char* CALIBRATION_KEY = "calibration";

    /** NVS keys of flags, at most 15 characters. Never rename them. */
    constexpr const char *WIFI_ASKED_KEY = "wifiAsked";
    constexpr const char *ROLLBACK_SHOWN_KEY = "rollbackShown";

    /** NVS keys of the automatic updates, at most 15 characters. Never rename them. */
    // starts since the last successful check, a byte
    constexpr const char *UPDATE_STARTS_KEY = "updStarts";
    // the newer version the last check found, empty if none
    constexpr const char *UPDATE_FOUND_KEY = "updVersion";
    // the version an automatic update installed, until it works or is rolled back
    constexpr const char *UPDATE_INSTALLING_KEY = "updInstalling";
    // the version that was rolled back, never offered again
    constexpr const char *UPDATE_ROLLED_BACK_KEY = "updRolledBack";

    /**
     * Opens the settings, migrates the values of older firmware and reads the
     * calibration. Returns nullptr if all went well, otherwise the name of the
     * ESP error that kept the calibration from being read. Device only.
     */
    const char* begin();

    /** The stored value, or the setting's default if none is stored. */
    float getFloat(FloatSetting s);
    /** Stores the value right away. */
    void setFloat(FloatSetting s, float value);

    /** The stored level's index, the default if none is stored or it is out of range. */
    uint8_t getChoice(ChoiceSetting s);
    /** Stores the level's index right away. */
    void setChoice(ChoiceSetting s, uint8_t index);

    /** index if it is one of the setting's levels, otherwise the default. */
    static inline uint8_t validChoice(const ChoiceSetting &s, int index)
    {
        return index >= 0 && index < s.count ? index : s.def;
    }

    /** Grams a lift has to dip for the stored level, 0 if lift-to-tare is off. */
    static inline float liftTareDepth() { return LIFT_TARE_DEPTHS[getChoice(choiceSettings[LIFT_TARE])]; }

    /** The shot offset in ms, within 0–10 s, 0 if none is stored. */
    static inline uint32_t shotOffsetMs()
    {
        float seconds = getFloat(floatSettings[SHOT_OFFSET]);
        if (std::isnan(seconds) || seconds < 0)
        {
            return 0;
        }
        return (uint32_t)(std::fmin(seconds, 10.0f) * 1000);
    }

    /** The typical shot time in seconds, the default 30 s if outside 10–60 s or none is stored. */
    static inline float typicalShotSeconds()
    {
        const auto &setting = floatSettings[SHOT_TIME];
        float seconds = getFloat(setting);
        return seconds >= 10 && seconds <= 60 ? seconds : setting.def;
    }

    /** True if a shot detected in the scale mode switches to espresso. */
    static inline bool shotAutoSwitch() { return getChoice(choiceSettings[SHOT_AUTO]) == 1; }

    /** Grams per count, 1.0 if the scale was never calibrated. Device only. */
    float getCalibration();
    /** Stores the calibration right away. Device only. */
    void setCalibration(float gramsPerCount);

    /**
     * True if the flag is set, and also if the settings can't be read, so a
     * broken flash doesn't ask the same on every start. Device only.
     */
    bool getFlag(const char *key);
    /** Sets or clears the flag right away. Device only. */
    void setFlag(const char *key, bool value);

    /** The stored number, def if none is stored or the settings can't be read. Device only. */
    uint8_t getByte(const char *key, uint8_t def);
    /** Stores the number right away. Device only. */
    void setByte(const char *key, uint8_t value);

    /**
     * The stored text into out, empty if none is stored, it doesn't fit into
     * size, or the settings can't be read. Device only.
     */
    void getText(const char *key, char *out, size_t size);
    /** Stores the text right away. Device only. */
    void setText(const char *key, const char *value);

    /** The names of all settings, in the menu's order. */
    static inline const char** getOptions()
    {
        static const char* options[SETTING_COUNT];
        static bool initialized = false;

        if (!initialized)
        {
            for (int i = 0; i < FLOAT_SETTING_NUM; i++)
            {
                options[i] = floatSettings[i].name;
            }
            for (int i = 0; i < CHOICE_SETTING_NUM; i++)
            {
                options[FLOAT_SETTING_NUM + i] = choiceSettings[i].name;
            }
            initialized = true;
        }

        return options;
    }

    static std::vector<float> getAllAutoTares()
    {
        std::vector<float> autoTares;

        for (int i = AUTO_TARE_0; i <= AUTO_TARE_4; ++i)
        {
            float value = getFloat(floatSettings[i]);

            // only use values that arent NaN and greater than 1
            if (!std::isnan(value) && value > 1)
            {
                autoTares.push_back(value);
            }
        }

        return autoTares;
    }
}