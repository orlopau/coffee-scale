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
        FLOAT_SETTING_NUM
    };

    constexpr std::array<FloatSetting, FLOAT_SETTING_NUM> floatSettings = {{
        {AUTO_TARE_0, SETTINGS_TARE_0, NAN, 0.5f},
        {AUTO_TARE_1, SETTINGS_TARE_1, NAN, 0.5f},
        {AUTO_TARE_2, SETTINGS_TARE_2, NAN, 0.5f},
        {AUTO_TARE_3, SETTINGS_TARE_3, NAN, 0.5f},
        {AUTO_TARE_4, SETTINGS_TARE_4, NAN, 0.5f},
        {AUTO_TARE_TOLERANCE, SETTINGS_TARE_TOLERANCE, NAN, 0.1f},
        {AVERAGING_DELTA, SETTINGS_AVERAGING_DELTA, 1.0f, 0.1f},
    }};

    float getFloat(FloatSetting s);
    void setFloat(FloatSetting s, float value);
    void commit();

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