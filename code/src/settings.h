#pragma once

#include "data/localization.h"
#include <vector>
#include <cmath>

namespace Settings
{
    enum FloatSetting
    {
        AUTO_TARE_0 = 0,
        AUTO_TARE_1,
        AUTO_TARE_2,
        AUTO_TARE_3,
        AUTO_TARE_4,
        AUTO_TARE_TOLERANCE,
        FLOAT_SETTING_NUM
    };

    static const char *floatSettingNames[] = {
        SETTINGS_TARE_0, SETTINGS_TARE_1, SETTINGS_TARE_2, SETTINGS_TARE_3, SETTINGS_TARE_4, SETTINGS_TARE_TOLERANCE
    };

    float getFloat(FloatSetting s);
    void setFloat(FloatSetting s, float value);
    void commit();

    static std::vector<float> getAllAutoTares()
    {
        std::vector<float> autoTares;

        for (int i = AUTO_TARE_0; i <= AUTO_TARE_4; ++i)
        {
            FloatSetting setting = static_cast<FloatSetting>(i);
            float value = getFloat(setting);

            // only use values that arent NaN and greater than 1
            if (!std::isnan(value) && value > 1)
            {
                autoTares.push_back(value);
            }
        }

        return autoTares;
    }
}