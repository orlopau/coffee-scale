#ifndef NATIVE

#include <stdlib.h>
#include <string.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <mdns.h>

#include "embedded/network/dev_server_search.h"

#define TAG "DEVSEARCH"

namespace DevServerSearch
{
    // a TXT value is not null-terminated (its length is txt_value_len), and null for a key without one
    static std::string txtValue(const mdns_result_t *result, const char *key)
    {
        for (size_t i = 0; i < result->txt_count; i++)
        {
            if (strcmp(result->txt[i].key, key) == 0)
            {
                const char *value = result->txt[i].value;
                return value != nullptr ? std::string(value, result->txt_value_len[i]) : std::string();
            }
        }
        return "";
    }

    /** Starts mDNS once. Callers hold the search lock. */
    static bool start()
    {
        static bool started = false;
        if (started)
        {
            return true;
        }
        esp_err_t err = mdns_init();
        if (err == ESP_OK)
        {
            err = mdns_hostname_set("coffee-scale");
        }
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "mDNS could not be started: %s", esp_err_to_name(err));
            return false;
        }
        started = true;
        return true;
    }

    bool find(Server &server, uint32_t timeoutMs)
    {
        // the updater and the log's task may search at once, and start mDNS only once
        static SemaphoreHandle_t lock = xSemaphoreCreateMutex();
        xSemaphoreTake(lock, portMAX_DELAY);

        bool found = false;
        mdns_result_t *results = nullptr;
        if (start() && mdns_query_ptr("_coffeescale-fw", "_tcp", timeoutMs, 20, &results) == ESP_OK)
        {
            for (const mdns_result_t *result = results; result != nullptr && !found; result = result->next)
            {
                for (const mdns_ip_addr_t *addr = result->addr; addr != nullptr; addr = addr->next)
                {
                    if (addr->addr.type != ESP_IPADDR_TYPE_V4)
                    {
                        continue;
                    }
                    server.ip = addr->addr.u_addr.ip4;
                    server.port = result->port;
                    server.firmwarePath = txtValue(result, "path");
                    server.recordPath = txtValue(result, "record");
                    const int logPort = atoi(txtValue(result, "log").c_str());
                    server.logPort = logPort > 0 && logPort <= 65535 ? logPort : 0;
                    found = true;
                    break;
                }
            }
        }
        mdns_query_results_free(results);
        xSemaphoreGive(lock);
        return found;
    }
}

#endif
