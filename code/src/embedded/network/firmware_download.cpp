#ifndef NATIVE

#include <esp_err.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_ota_ops.h>

#include "embedded/network/firmware_download.h"

// from ESP-IDF's esp_crt_bundle.h, in the prebuilt libmbedtls.a with the full
// Mozilla bundle. Not included: WiFiClientSecure, which the recorder's
// HTTPClient pulls in, has a header of the same name earlier on the path.
extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

#define TAG "DOWNLOAD"

namespace FirmwareDownload
{
    static const int MAX_REDIRECTS = 5;
    static const size_t BUFFER = 4096;
    // results of open() besides a length
    static const int64_t UNKNOWN_LENGTH = -1;
    static const int64_t NOT_MODIFIED = -2;
    static const int64_t FAILED = -3;

    static char buffer[BUFFER];

    /** Requests the URL and follows its redirects. The content length, or one of the results above. */
    static int64_t open(esp_http_client_handle_t client)
    {
        for (int redirects = 0; redirects <= MAX_REDIRECTS; redirects++)
        {
            esp_err_t err = esp_http_client_open(client, 0);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Connecting failed: %s", esp_err_to_name(err));
                return FAILED;
            }
            const int64_t length = esp_http_client_fetch_headers(client);
            const int status = esp_http_client_get_status_code(client);
            ESP_LOGI(TAG, "HTTP %d, %lld bytes", status, (long long)length);
            if (status == 200)
            {
                return length > 0 ? length : UNKNOWN_LENGTH;
            }
            if (status == 304)
            {
                return NOT_MODIFIED;
            }
            if (status < 300 || status >= 400 || esp_http_client_set_redirection(client) != ESP_OK)
            {
                ESP_LOGE(TAG, "Unexpected HTTP %d", status);
                return FAILED;
            }
            // the redirect's body, before the next request on this connection
            while (esp_http_client_read(client, buffer, BUFFER) > 0)
            {
            }
        }
        ESP_LOGE(TAG, "Too many redirects");
        return FAILED;
    }

    /** Writes the body into partition. */
    static bool install(esp_http_client_handle_t client, const esp_partition_t *partition, int64_t length,
                        Progress progress)
    {
        esp_ota_handle_t ota = 0;
        // erases as it writes, instead of the whole partition up front
        esp_err_t err = esp_ota_begin(partition, OTA_WITH_SEQUENTIAL_WRITES, &ota);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Starting the update failed: %s", esp_err_to_name(err));
            return false;
        }
        size_t done = 0;
        for (;;)
        {
            const int read = esp_http_client_read(client, buffer, BUFFER);
            if (read < 0)
            {
                ESP_LOGE(TAG, "Reading failed after %u bytes", (unsigned)done);
                esp_ota_abort(ota);
                return false;
            }
            if (read == 0)
            {
                break;
            }
            err = esp_ota_write(ota, buffer, read);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Writing failed: %s", esp_err_to_name(err));
                esp_ota_abort(ota);
                return false;
            }
            done += read;
            progress(done, length > 0 ? (size_t)length : 0);
        }
        if (length > 0 && (int64_t)done != length)
        {
            ESP_LOGE(TAG, "Got %u of %lld bytes", (unsigned)done, (long long)length);
            esp_ota_abort(ota);
            return false;
        }
        // checks the whole image, its checksum and hash
        err = esp_ota_end(ota);
        if (err == ESP_OK)
        {
            err = esp_ota_set_boot_partition(partition);
        }
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Installing failed: %s", esp_err_to_name(err));
            return false;
        }
        return true;
    }

    Result run(const char *url, Progress progress, const char *runningMd5)
    {
        const esp_partition_t *partition = esp_ota_get_next_update_partition(nullptr);
        if (partition == nullptr)
        {
            ESP_LOGE(TAG, "No partition to update into");
            return Result::Failed;
        }

        esp_http_client_config_t config = {};
        config.url = url;
        config.timeout_ms = 15000;
        config.buffer_size = BUFFER;
        // GitHub redirects to its asset host with a URL of over 500 bytes
        config.buffer_size_tx = BUFFER;
        // only used for https://
        config.crt_bundle_attach = esp_crt_bundle_attach;
        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == nullptr)
        {
            return Result::Failed;
        }
        esp_http_client_set_header(client, "x-ESP32-sketch-md5", runningMd5);

        Result result = Result::Failed;
        const int64_t length = open(client);
        if (length == NOT_MODIFIED)
        {
            result = Result::NoUpdate;
        }
        else if (length != FAILED && install(client, partition, length, progress))
        {
            result = Result::Installed;
        }
        esp_http_client_cleanup(client);
        return result;
    }
}

#endif
