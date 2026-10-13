#ifndef NATIVE

#include <esp_err.h>
#include <esp_http_client.h>
#include <esp_log.h>

#include "firmware_version.h"
#include "embedded/network/release_check.h"

// from ESP-IDF's esp_crt_bundle.h, see firmware_download.cpp for why it isn't included
extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

#define TAG "RELEASE"

namespace ReleaseCheck
{
    // GitHub's headers are long, e.g. its Content-Security-Policy
    static const int BUFFER = 4096;
    // per step (connecting, TLS, headers); short, as the check holds Wi-Fi meanwhile
    static const int TIMEOUT_MS = 5000;
    // a moved repository redirects once more than GitHub's usual one
    static const int MAX_REDIRECTS = 5;
    // GitHub's redirects are under 200 bytes; only the check's task uses it
    static char location[512];

    bool latestVersion(const char *url, char *version, size_t size)
    {
        esp_http_client_config_t config = {};
        config.url = url;
        config.method = HTTP_METHOD_HEAD;
        config.timeout_ms = TIMEOUT_MS;
        config.buffer_size = BUFFER;
        // only used for https://
        config.crt_bundle_attach = esp_crt_bundle_attach;
        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == nullptr)
        {
            return false;
        }

        bool found = false;
        for (int redirects = 0; redirects < MAX_REDIRECTS && !found; redirects++)
        {
            esp_err_t err = esp_http_client_open(client, 0);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Connecting failed: %s", esp_err_to_name(err));
                break;
            }
            esp_http_client_fetch_headers(client);
            const int status = esp_http_client_get_status_code(client);
            // set_redirection() takes the Location header, resolved against the address before
            if (status < 300 || status >= 400 || esp_http_client_set_redirection(client) != ESP_OK ||
                esp_http_client_get_url(client, location, sizeof(location)) != ESP_OK)
            {
                ESP_LOGE(TAG, "Expected a redirect, got HTTP %d", status);
                break;
            }
            found = FirmwareVersion::fromRedirect(location, version, size);
            // the next request goes to location; a HEAD answer has no body to read past
            esp_http_client_close(client);
        }
        if (!found)
        {
            ESP_LOGE(TAG, "No release found from %s", url);
        }
        esp_http_client_cleanup(client);
        return found;
    }
}

#endif
