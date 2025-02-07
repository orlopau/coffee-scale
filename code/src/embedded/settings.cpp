#ifndef NATIVE

#include <EEPROM.h>

#include "settings.h"
#include "constants.h"

namespace Settings
{
    float getFloat(FloatSetting s)
    {
        float value;
        EEPROM.get(EEPROM_ADDR_FLOAT_SETTINGS + sizeof(float) * s.id, value);
        if (isnan(value))
        {
            return s.def;
        }
        return value;
    }

    void setFloat(FloatSetting s, float value) {
        EEPROM.put(EEPROM_ADDR_FLOAT_SETTINGS + sizeof(float) * s.id, value);
    }

    void commit() {
        EEPROM.commit();
    }
}

#endif