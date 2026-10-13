#ifndef NATIVE

#include <math.h>
#include <stdint.h>
#include <string.h>

#include <vector>

#include <esp_err.h>
// Arduino's IDF only compiles in errors, and without Arduino.h the info
// logs of the migration would be left out
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>
#include <nvs.h>

#include "settings.h"

#define TAG "SETTINGS"

namespace Settings
{
    static const char *NAMESPACE = "settings";
    static const float CALIBRATION_DEFAULT = 1.0f;

    // Up to this firmware the values were stored with Arduino's EEPROM
    // emulation: one blob in NVS, a float at a fixed offset per value.
    static const char *OLD_NAMESPACE = "eeprom";
    static const char *OLD_BLOB = "eeprom";
    static const size_t OLD_ADDR_CALIBRATION = 0;
    static const size_t OLD_ADDR_FLOAT_SETTINGS = 10;

    // The C API, not nvs_handle.hpp: only the C functions take NVS's lock, and
    // the settings are used from the automatic updates' task and by the Wi-Fi
    // driver on the other core while the loop runs.
    static nvs_handle_t handle;
    // false if the settings couldn't be opened: getters return defaults, setters do nothing
    static bool isOpen = false;

    static esp_err_t readFloat(const char *key, float &value)
    {
        if (!isOpen)
        {
            return ESP_ERR_NVS_INVALID_HANDLE;
        }
        size_t size = sizeof(value);
        esp_err_t err = nvs_get_blob(handle, key, &value, &size);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Reading %s failed: %s", key, esp_err_to_name(err));
        }
        return err;
    }

    static esp_err_t writeFloat(const char *key, float value)
    {
        if (!isOpen)
        {
            ESP_LOGE(TAG, "Not saving %s, the settings aren't open", key);
            return ESP_ERR_NVS_INVALID_HANDLE;
        }
        esp_err_t err = nvs_set_blob(handle, key, &value, sizeof(value));
        if (err == ESP_OK)
        {
            err = nvs_commit(handle);
        }
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Saving %s failed: %s", key, esp_err_to_name(err));
        }
        return err;
    }

    /**
     * Copies the values of the old EEPROM blob to their keys, then erases the
     * blob. The missing blob marks the migration as done. If a write fails,
     * the blob stays, and the next start tries again.
     *
     * Returns the error of copying the calibration, so a calibration that is
     * still in the old blob doesn't look like a scale never calibrated.
     */
    static esp_err_t migrateEeprom()
    {
        esp_err_t err;
        std::vector<uint8_t> blob;
        {
            // read-only, so a scale without the old blob doesn't get the namespace created
            nvs_handle_t old;
            if (nvs_open(OLD_NAMESPACE, NVS_READONLY, &old) != ESP_OK)
            {
                return ESP_OK;
            }
            size_t size = 0;
            if (nvs_get_blob(old, OLD_BLOB, nullptr, &size) != ESP_OK || size == 0)
            {
                nvs_close(old);
                return ESP_OK;
            }
            blob.resize(size);
            err = nvs_get_blob(old, OLD_BLOB, blob.data(), &size);
            nvs_close(old);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Reading the old EEPROM failed: %s", esp_err_to_name(err));
                return err;
            }
        }

        // NaN (0xFF in a fresh EEPROM) means the value was never set
        auto copy = [&blob](size_t addr, const char *key) {
            float value;
            if (addr + sizeof(value) > blob.size())
            {
                return ESP_OK;
            }
            memcpy(&value, &blob[addr], sizeof(value));
            if (isnan(value))
            {
                return ESP_OK;
            }
            ESP_LOGI(TAG, "Migrating %s: %f", key, value);
            return writeFloat(key, value);
        };

        err = copy(OLD_ADDR_CALIBRATION, CALIBRATION_KEY);
        if (err != ESP_OK)
        {
            return err;
        }
        for (const auto &setting : floatSettings)
        {
            // the calibration is copied already, the settings fall back to their defaults
            if (copy(OLD_ADDR_FLOAT_SETTINGS + sizeof(float) * setting.id, setting.key) != ESP_OK)
            {
                return ESP_OK;
            }
        }

        nvs_handle_t old;
        err = nvs_open(OLD_NAMESPACE, NVS_READWRITE, &old);
        if (err == ESP_OK)
        {
            err = nvs_erase_key(old, OLD_BLOB);
            if (err == ESP_OK)
            {
                err = nvs_commit(old);
            }
            nvs_close(old);
        }
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Erasing the old EEPROM failed: %s", esp_err_to_name(err));
            return ESP_OK;
        }
        ESP_LOGI(TAG, "Migrated the old EEPROM");
        return ESP_OK;
    }

    const char *begin()
    {
        // the runtime level is errors only, too
        esp_log_level_set(TAG, ESP_LOG_INFO);

        esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
        isOpen = err == ESP_OK;
        if (!isOpen)
        {
            ESP_LOGE(TAG, "Opening the settings failed: %s", esp_err_to_name(err));
            return esp_err_to_name(err);
        }

        err = migrateEeprom();
        if (err != ESP_OK)
        {
            return esp_err_to_name(err);
        }

        // not found: never calibrated, not an error
        float calibration;
        err = readFloat(CALIBRATION_KEY, calibration);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
        {
            return esp_err_to_name(err);
        }
        return nullptr;
    }

    float getFloat(FloatSetting s)
    {
        float value;
        if (readFloat(s.key, value) != ESP_OK || isnan(value))
        {
            return s.def;
        }
        return value;
    }

    void setFloat(FloatSetting s, float value) { writeFloat(s.key, value); }

    uint8_t getChoice(ChoiceSetting s)
    {
        if (!isOpen)
        {
            return s.def;
        }
        uint8_t index;
        esp_err_t err = nvs_get_u8(handle, s.key, &index);
        if (err != ESP_OK)
        {
            if (err != ESP_ERR_NVS_NOT_FOUND)
            {
                ESP_LOGE(TAG, "Reading %s failed: %s", s.key, esp_err_to_name(err));
            }
            return s.def;
        }
        return validChoice(s, index);
    }

    void setChoice(ChoiceSetting s, uint8_t index) { setByte(s.key, index); }

    float getCalibration()
    {
        float value;
        if (readFloat(CALIBRATION_KEY, value) != ESP_OK || isnan(value))
        {
            return CALIBRATION_DEFAULT;
        }
        return value;
    }

    void setCalibration(float gramsPerCount) { writeFloat(CALIBRATION_KEY, gramsPerCount); }

    bool getFlag(const char *key)
    {
        if (!isOpen)
        {
            return true;
        }
        uint8_t value;
        esp_err_t err = nvs_get_u8(handle, key, &value);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            return false;
        }
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Reading %s failed: %s", key, esp_err_to_name(err));
            return true;
        }
        return value != 0;
    }

    void setFlag(const char *key, bool value) { setByte(key, value ? 1 : 0); }

    uint8_t getByte(const char *key, uint8_t def)
    {
        if (!isOpen)
        {
            return def;
        }
        uint8_t value;
        esp_err_t err = nvs_get_u8(handle, key, &value);
        if (err != ESP_OK)
        {
            if (err != ESP_ERR_NVS_NOT_FOUND)
            {
                ESP_LOGE(TAG, "Reading %s failed: %s", key, esp_err_to_name(err));
            }
            return def;
        }
        return value;
    }

    void setByte(const char *key, uint8_t value)
    {
        if (!isOpen)
        {
            ESP_LOGE(TAG, "Not saving %s, the settings aren't open", key);
            return;
        }
        esp_err_t err = nvs_set_u8(handle, key, value);
        if (err == ESP_OK)
        {
            err = nvs_commit(handle);
        }
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Saving %s failed: %s", key, esp_err_to_name(err));
        }
    }

    void getText(const char *key, char *out, size_t size)
    {
        if (size == 0)
        {
            return;
        }
        out[0] = '\0';
        if (!isOpen)
        {
            return;
        }
        esp_err_t err = nvs_get_str(handle, key, out, &size);
        if (err != ESP_OK)
        {
            out[0] = '\0';
            if (err != ESP_ERR_NVS_NOT_FOUND)
            {
                ESP_LOGE(TAG, "Reading %s failed: %s", key, esp_err_to_name(err));
            }
        }
    }

    void setText(const char *key, const char *value)
    {
        if (!isOpen)
        {
            ESP_LOGE(TAG, "Not saving %s, the settings aren't open", key);
            return;
        }
        esp_err_t err = nvs_set_str(handle, key, value);
        if (err == ESP_OK)
        {
            err = nvs_commit(handle);
        }
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Saving %s failed: %s", key, esp_err_to_name(err));
        }
    }
}

#endif
