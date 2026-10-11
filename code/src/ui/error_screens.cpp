#include <stdio.h>
#include <string.h>

#include "ui/error_screens.h"
#include "ui/widgets.h"
#include "data/localization.h"

namespace ErrorScreens
{
    void settingsUnreadable(Canvas &canvas, const char *errorName)
    {
        static const char NVS_PREFIX[] = "ESP_ERR_NVS_";
        if (strncmp(errorName, NVS_PREFIX, sizeof(NVS_PREFIX) - 1) == 0)
        {
            errorName += sizeof(NVS_PREFIX) - 1;
        }

        char text[96];
        snprintf(text, sizeof(text), SETTINGS_UNREADABLE, errorName);
        Widgets::textLines(canvas, text);
    }
}
