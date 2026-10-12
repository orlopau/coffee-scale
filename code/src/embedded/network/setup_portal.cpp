#ifndef NATIVE

#include <stdio.h>

#include <string>

#include <esp_http_server.h>
#include <esp_log.h>

#include "data/setup_page.h"
#include "embedded/network/setup_portal.h"
#include "embedded/network/wifi_station.h"
#include "network/setup_web.h"

#define TAG "PORTAL"

namespace SetupPortal
{
    // both fields, percent-encoded, are well below this
    static const size_t MAX_BODY = 512;
    static const size_t MAX_SSID = 32;
    static const size_t MAX_PASSWORD = 64;

    static httpd_handle_t server = nullptr;
    static SetupState *setupState = nullptr;
    static char location[32];

    static esp_err_t sendJson(httpd_req_t *request, const std::string &json)
    {
        httpd_resp_set_type(request, "application/json");
        httpd_resp_set_hdr(request, "Cache-Control", "no-store");
        return httpd_resp_send(request, json.c_str(), json.size());
    }

    static esp_err_t page(httpd_req_t *request)
    {
        httpd_resp_set_type(request, "text/html; charset=utf-8");
        httpd_resp_set_hdr(request, "Cache-Control", "no-store");
        return httpd_resp_send(request, SETUP_PAGE, HTTPD_RESP_USE_STRLEN);
    }

    static esp_err_t scan(httpd_req_t *request)
    {
        return sendJson(request, SetupWeb::networksJson(WifiStation::scan()));
    }

    static esp_err_t status(httpd_req_t *request)
    {
        return sendJson(request, std::string("{\"state\":\"") + wifiStatusName(setupState->status()) + "\"}");
    }

    static esp_err_t connect(httpd_req_t *request)
    {
        if (request->content_len > MAX_BODY)
        {
            httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, "Too long");
            return ESP_FAIL;
        }
        char body[MAX_BODY];
        size_t received = 0;
        int timeouts = 0;
        while (received < request->content_len)
        {
            const int length = httpd_req_recv(request, body + received, request->content_len - received);
            if (length == HTTPD_SOCK_ERR_TIMEOUT && ++timeouts < 3)
            {
                continue;
            }
            if (length <= 0)
            {
                return ESP_FAIL;
            }
            received += length;
        }

        std::string ssid, password;
        if (!SetupWeb::formField(body, received, "ssid", ssid, MAX_SSID) || ssid.empty() ||
            !SetupWeb::formField(body, received, "password", password, MAX_PASSWORD))
        {
            httpd_resp_send_err(request, HTTPD_400_BAD_REQUEST, "No network");
            return ESP_FAIL;
        }
        // the page polls /status either way
        httpd_resp_set_status(request, setupState->request(ssid, password) ? "202 Accepted" : "409 Conflict");
        return httpd_resp_send(request, nullptr, 0);
    }

    static esp_err_t redirect(httpd_req_t *request)
    {
        httpd_resp_set_status(request, "302 Found");
        httpd_resp_set_hdr(request, "Location", location);
        return httpd_resp_send(request, nullptr, 0);
    }

    bool start(SetupState &state)
    {
        if (server != nullptr)
        {
            return true;
        }
        setupState = &state;
        const esp_ip4_addr_t ip = WifiStation::accessPointIp();
        snprintf(location, sizeof(location), "http://" IPSTR "/", IP2STR(&ip));

        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.uri_match_fn = httpd_uri_match_wildcard;
        // phones open many connections while they check for internet
        config.lru_purge_enable = true;
        // the network list is put together in std::strings
        config.stack_size = 8192;
        if (httpd_start(&server, &config) != ESP_OK)
        {
            ESP_LOGE(TAG, "Starting the server failed");
            server = nullptr;
            return false;
        }

        // the first matching handler answers, so the catch-all comes last
        const httpd_uri_t handlers[] = {
            {"/", HTTP_GET, page, nullptr},
            {"/scan", HTTP_GET, scan, nullptr},
            {"/status", HTTP_GET, status, nullptr},
            {"/connect", HTTP_POST, connect, nullptr},
            {"/*", HTTP_GET, redirect, nullptr},
        };
        for (const httpd_uri_t &handler : handlers)
        {
            httpd_register_uri_handler(server, &handler);
        }
        return true;
    }

    void stop()
    {
        if (server != nullptr)
        {
            httpd_stop(server);
            server = nullptr;
        }
    }
}

#endif
