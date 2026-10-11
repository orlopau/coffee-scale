#include "settings.h"

namespace Settings
{
    // like a device with no settings saved: 0 for everything not set
    static std::array<float, FLOAT_SETTING_NUM> values = {};

    float getFloat(FloatSetting s) { return values[s.id]; }

    void setFloat(FloatSetting s, float value) { values[s.id] = value; }
}
