#ifndef NATIVE

#include <esp_log.h>
#include <esp_ota_ops.h>

#include "embedded/network/firmware_check.h"
#include "settings.h"

#define TAG "FIRMWARE"

// The core marks every firmware working right at boot, unless this returns
// true (esp32-hal-misc.c, weak). Then FirmwareCheck::markValid() does, once
// the scale really runs.
extern "C" bool verifyRollbackLater() { return true; }

namespace FirmwareCheck
{
    bool onProbation()
    {
        esp_ota_img_states_t state;
        return esp_ota_get_state_partition(esp_ota_get_running_partition(), &state) == ESP_OK &&
               state == ESP_OTA_IMG_PENDING_VERIFY;
    }

    void markValid()
    {
        // e.g. flashed over USB: nothing to confirm
        if (!onProbation())
        {
            return;
        }
        esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "The new firmware works, no rollback");
        }
        else
        {
            ESP_LOGE(TAG, "Marking the firmware as working failed: %s", esp_err_to_name(err));
        }
    }

    bool takeRolledBack()
    {
        // the partition stays invalid until the next update overwrites it, so a flag says it was shown
        if (esp_ota_get_last_invalid_partition() == nullptr || Settings::getFlag(Settings::ROLLBACK_SHOWN_KEY))
        {
            return false;
        }
        Settings::setFlag(Settings::ROLLBACK_SHOWN_KEY, true);
        return true;
    }

    void expectNewFirmware() { Settings::setFlag(Settings::ROLLBACK_SHOWN_KEY, false); }
}

#endif
