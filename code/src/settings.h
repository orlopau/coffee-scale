#pragma once

#include "data/localization.h"
#include <vector>
#include <cmath>
#include <array>

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
    }};

    /** NVS key of the calibration, in grams per count. */
    constexpr const char* CALIBRATION_KEY = "calibration";

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

    /** Grams per count, 1.0 if the scale was never calibrated. Device only. */
    float getCalibration();
    /** Stores the calibration right away. Device only. */
    void setCalibration(float gramsPerCount);

    static inline const char** getOptions()
    {
        static const char* options[FLOAT_SETTING_NUM];
        static bool initialized = false;

        if (!initialized)
        {
            for (int i = 0; i < FLOAT_SETTING_NUM; i++)
            {
                options[i] = floatSettings[i].name;
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