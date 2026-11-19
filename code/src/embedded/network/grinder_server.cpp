#ifndef NATIVE

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <freertos/task.h>
#include <mdns.h>

#include "embedded/network/grinder_server.h"
#include "embedded/network/wifi_station.h"
#include "grinder/reconnect_backoff.h"
#include "millis.h"

#define TAG "GRINDER"

namespace GrinderServer
{
    // longer bodies are refused; the grinder's are a few hundred bytes
    static const size_t MAX_BODY = 4096;
    // the task looks at the setting and the connection this often
    static const uint32_t TICK_MS = 2000;
    // like the updater
    static const uint32_t CONNECT_TIMEOUT_MS = 15000;
    // the connect and mDNS calls run on it
    static const uint32_t TASK_STACK = 6144;
    // below the button (2) and load cell (5) tasks
    static const UBaseType_t TASK_PRIORITY = 1;
    // with Wi-Fi; the loop, the load cell and the button tasks run on core 1
    static const BaseType_t TASK_CORE = 0;
    // receive timeouts in a row before a body is given up
    static const int MAX_TIMEOUTS = 3;
    // the reply is built from floats and strings
    static const size_t SERVER_STACK = 6144;
    // the server answers one request at a time, so one buffer is enough
    // allocated while the server runs, so the setting Off costs no RAM for it
    static char *body = nullptr;

    static Xenia::Machine *machine = nullptr;
    // guards *machine, taken only around the calls on it
    static SemaphoreHandle_t machineLock = nullptr;
    // guards the server, mDNS and the radio below: the task starts them, pause() stops them
    static SemaphoreHandle_t stateLock = nullptr;

    static volatile bool enabled = false;
    static volatile bool paused = false;
    static volatile uint32_t lastPoll = 0;

    // under stateLock
    static httpd_handle_t server = nullptr;
    static bool mdnsOn = false;
    // the address the Xenia's names point to, 0 while they are not announced
    static uint32_t xeniaNamesIp = 0;

    // A Xenia answers as xenia-coffee.local (its manual) or xenia.local (its owners): perhaps the grinder's
    // scan asks for one of them. Announced only with the setting on, like everything here.
    static const char *const XENIA_NAMES[] = {"xenia-coffee", "xenia"};
    // keeps Wi-Fi on while the server runs or is wanted. The IDF station
    // default power save (WIFI_PS_MIN_MODEM) stays: it wakes at every DTIM
    // beacon, the grinder polls every 2 s and can wait that long.
    static WifiStation::Radio *radio = nullptr;

    // the connection attempts back off, see ReconnectBackoff. Only the task uses
    // these; setEnabled() and resume() ask for a fresh start with retryNow.
    static uint32_t retryWait = ReconnectBackoff::FIRST_MS;
    static uint32_t lastFailure = 0;
    static bool failed = false;
    static volatile bool retryNow = false;

    static bool wanted()
    {
        return enabled && !paused;
    }

    static esp_err_t send(httpd_req_t *request, const std::string &json)
    {
        httpd_resp_set_type(request, "application/json");
        return httpd_resp_send(request, json.c_str(), json.size());
    }

    /** The 10 characters the grinder takes as the machine's serial: CS and the end of the station's MAC. */
    static void serial(char *out, size_t size)
    {
        uint8_t mac[6] = {};
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        snprintf(out, size, "CS%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
    }

    /** Reads the request's body into body. False if it breaks off. */
    static bool readBody(httpd_req_t *request)
    {
        size_t received = 0;
        int timeouts = 0;
        while (received < request->content_len)
        {
            const int length = httpd_req_recv(request, body + received, request->content_len - received);
            // a stalled client must not keep the server's task, and so httpd_stop(), waiting
            if (length == HTTPD_SOCK_ERR_TIMEOUT && ++timeouts <= MAX_TIMEOUTS)
            {
                continue;
            }
            if (length <= 0)
            {
                return false;
            }
            received += length;
        }
        body[received] = '\0';
        return true;
    }

    static esp_err_t handle(httpd_req_t *request)
    {
        if (request->content_len > MAX_BODY)
        {
            // IDF 4.4 has no HTTPD_413 error code
            httpd_resp_set_status(request, "413 Payload Too Large");
            httpd_resp_set_hdr(request, "Connection", "close");
            send(request, "{}");
            // the body stays unread: failing closes the session
            return ESP_FAIL;
        }
        if (!readBody(request))
        {
            return ESP_FAIL;
        }

        const Xenia::Request kind = Xenia::classify(http_method_str((httpd_method_t)request->method), request->uri);
        std::string reply = "{}";
        // only the calls on the machine are locked, never the sending
        switch (kind)
        {
        case Xenia::Request::Mako:
            xSemaphoreTake(machineLock, portMAX_DELAY);
            reply = machine->mako(now());
            xSemaphoreGive(machineLock);
            lastPoll = now();
            break;
        case Xenia::Request::Identity:
        {
            char sn[11];
            serial(sn, sizeof(sn));
            reply = Xenia::identityJson(sn);
            break;
        }
        case Xenia::Request::BrewRatio:
        {
            const float grams = Xenia::brewWeightFrom(body);
            if (!isnan(grams))
            {
                xSemaphoreTake(machineLock, portMAX_DELAY);
                machine->setBrewWeight(grams);
                xSemaphoreGive(machineLock);
            }
            break;
        }
        case Xenia::Request::StartBrew:
            xSemaphoreTake(machineLock, portMAX_DELAY);
            machine->start(now());
            xSemaphoreGive(machineLock);
            break;
        case Xenia::Request::Other:
            break;
        }
        return send(request, reply);
    }

    /** Under stateLock. */
    static bool startServer()
    {
        body = static_cast<char *>(malloc(MAX_BODY + 1));
        if (body == nullptr)
        {
            ESP_LOGE(TAG, "No memory for the request body");
            return false;
        }
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.uri_match_fn = httpd_uri_match_wildcard;
        // the grinder may open a new connection for every poll
        config.lru_purge_enable = true;
        config.stack_size = SERVER_STACK;
        // the load cell task runs on core 1: keep the Wi-Fi work off its core
        config.core_id = 0;
        if (httpd_start(&server, &config) != ESP_OK)
        {
            ESP_LOGE(TAG, "Starting the server failed");
            server = nullptr;
            free(body);
            body = nullptr;
            return false;
        }
        const httpd_uri_t handlers[] = {
            {"/*", HTTP_GET, handle, nullptr},
            {"/*", HTTP_POST, handle, nullptr},
        };
        for (const httpd_uri_t &handler : handlers)
        {
            if (httpd_register_uri_handler(server, &handler) != ESP_OK)
            {
                // the next tick tries again
                ESP_LOGE(TAG, "Registering a handler failed");
                httpd_stop(server);
                server = nullptr;
                free(body);
                body = nullptr;
                return false;
            }
        }
        ESP_LOGI(TAG, "Serving the grinder");
        return true;
    }

    /** Under stateLock. */
    static void startMdns()
    {
        // update.cpp's MDNS.begin runs only in the boot updater, before this
        if (mdns_init() != ESP_OK)
        {
            ESP_LOGE(TAG, "Starting mDNS failed");
            return;
        }
        if (mdns_hostname_set("coffee-scale") != ESP_OK || mdns_instance_name_set("Coffee Scale") != ESP_OK ||
            mdns_service_add(nullptr, "_http", "_tcp", 80, nullptr, 0) != ESP_OK)
        {
            // the next tick starts it again
            ESP_LOGE(TAG, "Setting up mDNS failed");
            mdns_free();
            return;
        }
        mdnsOn = true;
    }

    /** Under stateLock, with mDNS on: points the Xenia's names to the current address, also after it changed. */
    static void updateXeniaNames()
    {
        esp_netif_t *station = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_ip_info_t info = {};
        if (station == nullptr || esp_netif_get_ip_info(station, &info) != ESP_OK || info.ip.addr == 0 ||
            info.ip.addr == xeniaNamesIp)
        {
            return;
        }
        mdns_ip_addr_t address = {};
        address.addr.type = ESP_IPADDR_TYPE_V4;
        address.addr.u_addr.ip4.addr = info.ip.addr;
        address.next = nullptr;
        bool ok = true;
        for (const char *name : XENIA_NAMES)
        {
            // a delegated name holds a fixed address: a new one replaces it; removing one not there only fails
            mdns_delegate_hostname_remove(name);
            ok =mdns_delegate_hostname_add(name, &address) == ESP_OK && ok;
        }
        // on a failure the next tick tries again
        xeniaNamesIp = ok ? info.ip.addr : 0;
        if (!ok)
        {
            ESP_LOGE(TAG, "Announcing the Xenia's names failed");
        }
    }

    /** Under stateLock: server, name and Wi-Fi all off. */
    static void stopAll()
    {
        if (server != nullptr)
        {
            // httpd_stop() returns after the handlers are done, so none reads body any more
            httpd_stop(server);
            server = nullptr;
            free(body);
            body = nullptr;
        }
        if (mdnsOn)
        {
            // also ends the Xenia's names
            mdns_free();
            mdnsOn = false;
            xeniaNamesIp = 0;
        }
        delete radio;
        radio = nullptr;
    }

    static void tick()
    {
        xSemaphoreTake(stateLock, portMAX_DELAY);
        if (!wanted())
        {
            stopAll();
            xSemaphoreGive(stateLock);
            return;
        }
        if (radio == nullptr)
        {
            radio = new WifiStation::Radio();
        }
        xSemaphoreGive(stateLock);

        if (retryNow)
        {
            retryNow = false;
            failed = false;
            retryWait = ReconnectBackoff::FIRST_MS;
        }
        // connected by whatever means, also by itself: the next failure starts at the first wait
        if (WifiStation::connected())
        {
            failed = false;
            retryWait = ReconnectBackoff::FIRST_MS;
        }
        // after a failure the next attempt waits; the tick itself goes on
        else if (!failed || ReconnectBackoff::over(lastFailure, retryWait, now()))
        {
            // not under the lock: it takes seconds, and pause() must not wait for it
            {
                // the scope ends the Online before connected() is read; the radio keeps Wi-Fi on
                WifiStation::Online online(CONNECT_TIMEOUT_MS);
            }
            if (!WifiStation::connected())
            {
                // the first failure waits the first time, every further one doubles it
                retryWait = failed ? ReconnectBackoff::next(retryWait) : ReconnectBackoff::FIRST_MS;
                failed = true;
                lastFailure = now();
                ESP_LOGI(TAG, "Not connected, next attempt in %u s", (unsigned)(retryWait / 1000));
            }
        }

        xSemaphoreTake(stateLock, portMAX_DELAY);
        // a pause or the setting may have come meanwhile
        if (wanted() && radio != nullptr && WifiStation::connected())
        {
            if (server == nullptr)
            {
                startServer();
            }
            if (!mdnsOn)
            {
                startMdns();
            }
            if (mdnsOn)
            {
                updateXeniaNames();
            }
        }
        xSemaphoreGive(stateLock);
    }

    static void run(void *)
    {
        for (;;)
        {
            tick();
            vTaskDelay(pdMS_TO_TICKS(TICK_MS));
        }
    }

    /** Once, on the first enabling: with the setting Off the task's stack stays free. It idles when off again. */
    static void startTask()
    {
        static bool started = false;
        if (started)
        {
            return;
        }
        if (xTaskCreatePinnedToCore(run, "grinderServer", TASK_STACK, nullptr, TASK_PRIORITY, nullptr, TASK_CORE) !=
            pdPASS)
        {
            ESP_LOGE(TAG, "Cannot start the task");
            return;
        }
        started = true;
    }

    void begin(Xenia::Machine &xenia, SemaphoreHandle_t lock)
    {
        if (machine != nullptr)
        {
            return;
        }
        machine = &xenia;
        machineLock = lock;
        stateLock = xSemaphoreCreateMutex();
        // the loop's task starts the driver, the task below may not be the first;
        // off, the driver stays shut down, setEnabled(true) starts it later
        if (enabled)
        {
            WifiStation::begin();
        }
        if (enabled)
        {
            startTask();
        }
    }

    void setEnabled(bool on)
    {
        if (on)
        {
            // from the loop's task, see begin()
            WifiStation::begin();
            retryNow = true;
        }
        enabled = on;
        if (on && stateLock != nullptr)
        {
            startTask();
        }
    }

    void pause()
    {
        paused = true;
        if (stateLock == nullptr)
        {
            return;
        }
        // the task may be mid-tick, then this waits for it and its next tick sees the pause
        xSemaphoreTake(stateLock, portMAX_DELAY);
        stopAll();
        xSemaphoreGive(stateLock);
    }

    void resume()
    {
        retryNow = true;
        paused = false;
    }

    void address(char *out, size_t size)
    {
        out[0] = '\0';
        esp_netif_t *station = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_ip_info_t ip = {};
        if (station != nullptr && WifiStation::connected() && esp_netif_get_ip_info(station, &ip) == ESP_OK)
        {
            snprintf(out, size, IPSTR, IP2STR(&ip.ip));
        }
    }

    uint32_t lastPollMs()
    {
        return lastPoll;
    }
}

#endif
