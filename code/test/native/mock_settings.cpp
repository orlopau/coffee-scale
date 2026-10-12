#include "settings.h"

namespace Settings
{
    // like a device with no settings saved: 0 for everything not set
    static std::array<float, FLOAT_SETTING_NUM> values = {};

    float getFloat(FloatSetting s) { return values[s.id]; }

    void setFloat(FloatSetting s, float value) { values[s.id] = value; }

    // -1: nothing stored, like a device with no settings saved
    static std::array<int, CHOICE_SETTING_NUM> choices = [] {
        std::array<int, CHOICE_SETTING_NUM> c;
        c.fill(-1);
        return c;
    }();

    uint8_t getChoice(ChoiceSetting s) { return validChoice(s, choices[s.id]); }

    void setChoice(ChoiceSetting s, uint8_t index) { choices[s.id] = index; }
}
