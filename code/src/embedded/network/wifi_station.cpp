#ifndef NATIVE

#include <string.h>

#include <dhcpserver/dhcpserver.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include "embedded/network/wifi_station.h"

#define TAG "WIFI"

static_assert(WIFI_REASON_AUTH_EXPIRE == 2 && WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT == 15 &&
                  WIFI_REASON_NO_AP_FOUND == 201 && WIFI_REASON_AUTH_FAIL == 202 &&
                  WIFI_REASON_HANDSHAKE_TIMEOUT == 204,
              "the reasons wifiStatusForDisconnect() knows");

namespace WifiStation
{
    static const EventBits_t GOT_IP = BIT0;
    static const EventBits_t DISCONNECTED = BIT1;
    // tries per connection, one failed try can be a missed beacon
    static const int ATTEMPTS = 3;
    // more networks than this don't fit on the page anyway
    static const uint16_t MAX_NETWORKS = 20;

    static bool started = false;
    static bool running = false;
    static bool accessPointOn = false;
    static EventGroupHandle_t events = nullptr;
    static esp_netif_t *accessPointNetif = nullptr;
    static volatile uint8_t lastReason = 0;

    static bool check(esp_err_t err, const char *what)
    {
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "%s failed: %s", what, esp_err_to_name(err));
        }
        return err == ESP_OK;
    }

    static void onEvent(void *, esp_event_base_t base, int32_t id, void *data)
    {
        if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED)
        {
            lastReason = ((wifi_event_sta_disconnected_t *)data)->reason;
            ESP_LOGI(TAG, "Disconnected, reason %u", lastReason);
            xEventGroupSetBits(events, DISCONNECTED);
        }
        else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP)
        {
            xEventGroupSetBits(events, GOT_IP);
        }
    }

    bool begin()
    {
        if (started)
        {
            return true;
        }
        if (!check(esp_netif_init(), "Starting the network stack"))
        {
            return false;
        }
        // it exists already if something else made it first
        esp_err_t err = esp_event_loop_create_default();
        if (err != ESP_ERR_INVALID_STATE && !check(err, "Creating the event loop"))
        {
            return false;
        }
        esp_netif_create_default_wifi_sta();
        accessPointNetif = esp_netif_create_default_wifi_ap();
        wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
        if (!check(esp_wifi_init(&config), "Starting Wi-Fi"))
        {
            return false;
        }
        // only a network that worked gets saved, see tryNetwork()
        esp_wifi_set_storage(WIFI_STORAGE_RAM);
        events = xEventGroupCreate();
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, onEvent, nullptr);
        esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, onEvent, nullptr);
        started = true;
        return true;
    }

    bool hasSavedNetwork()
    {
        wifi_config_t config = {};
        return started && esp_wifi_get_config(WIFI_IF_STA, &config) == ESP_OK && config.sta.ssid[0] != '\0';
    }

    static bool startRadio()
    {
        if (!running)
        {
            running = check(esp_wifi_start(), "Starting the radio");
        }
        return running;
    }

    /**
     * The station, next to the setup's network while that is open. Sets the
     * mode every time: ESP-IDF restores the last one from NVS, which may be
     * AutoConnect's with its open network.
     */
    static bool startStation()
    {
        if (!check(esp_wifi_set_mode(accessPointOn ? WIFI_MODE_APSTA : WIFI_MODE_STA), "Setting the mode"))
        {
            return false;
        }
        return startRadio();
    }

    /** Connects with the station's config, up to ATTEMPTS tries within timeoutMs. */
    static WifiStatus connectWithConfig(uint32_t timeoutMs)
    {
        const TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeoutMs);
        WifiStatus result = WifiStatus::Failed;
        for (int attempt = 0; attempt < ATTEMPTS; attempt++)
        {
            const TickType_t now = xTaskGetTickCount();
            if ((int32_t)(deadline - now) <= 0)
            {
                break;
            }
            xEventGroupClearBits(events, GOT_IP | DISCONNECTED);
            if (!check(esp_wifi_connect(), "Connecting"))
            {
                return WifiStatus::Failed;
            }
            const EventBits_t bits = xEventGroupWaitBits(events, GOT_IP | DISCONNECTED, pdTRUE, pdFALSE, deadline - now);
            if (bits & GOT_IP)
            {
                return WifiStatus::Connected;
            }
            if (!(bits & DISCONNECTED))
            {
                ESP_LOGI(TAG, "Connecting timed out");
                esp_wifi_disconnect();
                return WifiStatus::Failed;
            }
            result = wifiStatusForDisconnect(lastReason);
        }
        return result;
    }

    WifiStatus connect(uint32_t timeoutMs)
    {
        if (!hasSavedNetwork() || !startStation())
        {
            return WifiStatus::Failed;
        }
        return connectWithConfig(timeoutMs);
    }

    WifiStatus tryNetwork(const char *ssid, const char *password, uint32_t timeoutMs)
    {
        if (!started || !startStation())
        {
            return WifiStatus::Failed;
        }
        // back to it if this network doesn't work, so connect() still means the saved one
        wifi_config_t saved = {};
        esp_wifi_get_config(WIFI_IF_STA, &saved);
        // the fields aren't null-terminated when full: 32 bytes of name, 64 of password
        wifi_config_t config = {};
        strncpy((char *)config.sta.ssid, ssid, sizeof(config.sta.ssid));
        strncpy((char *)config.sta.password, password, sizeof(config.sta.password));
        if (!check(esp_wifi_set_config(WIFI_IF_STA, &config), "Setting the network"))
        {
            return WifiStatus::Failed;
        }
        const WifiStatus result = connectWithConfig(timeoutMs);
        if (result == WifiStatus::Connected)
        {
            esp_wifi_set_storage(WIFI_STORAGE_FLASH);
            check(esp_wifi_set_config(WIFI_IF_STA, &config), "Saving the network");
            esp_wifi_set_storage(WIFI_STORAGE_RAM);
        }
        else
        {
            check(esp_wifi_set_config(WIFI_IF_STA, &saved), "Restoring the saved network");
        }
        return result;
    }

    std::vector<SetupWeb::Network> scan()
    {
        std::vector<SetupWeb::Network> networks;
        // runs in the HTTP server's task: only while the setup has the radio on
        if (!running || !check(esp_wifi_scan_start(nullptr, true), "Scanning"))
        {
            return networks;
        }
        uint16_t count = 0;
        esp_wifi_scan_get_ap_num(&count);
        if (count > MAX_NETWORKS)
        {
            count = MAX_NETWORKS;
        }
        std::vector<wifi_ap_record_t> records(count);
        if (count > 0 && esp_wifi_scan_get_ap_records(&count, records.data()) != ESP_OK)
        {
            return networks;
        }
        for (uint16_t i = 0; i < count; i++)
        {
            const char *ssid = (const char *)records[i].ssid;
            networks.push_back({std::string(ssid, strnlen(ssid, sizeof(records[i].ssid))), records[i].rssi,
                                records[i].authmode != WIFI_AUTH_OPEN});
        }
        return networks;
    }

    bool startAccessPoint(const char *ssid, const char *password)
    {
        if (!started)
        {
            return false;
        }
        // not a private address: some Android versions only open the captive portal then
        esp_netif_ip_info_t ip = {};
        esp_netif_set_ip4_addr(&ip.ip, 4, 3, 2, 1);
        esp_netif_set_ip4_addr(&ip.gw, 4, 3, 2, 1);
        esp_netif_set_ip4_addr(&ip.netmask, 255, 255, 255, 0);
        // phones ask the scale for every name, see CaptiveDns
        esp_netif_dns_info_t dns = {};
        dns.ip.type = ESP_IPADDR_TYPE_V4;
        dns.ip.u_addr.ip4 = ip.ip;
        uint8_t offer = OFFER_DNS;
        esp_netif_dhcps_stop(accessPointNetif);
        check(esp_netif_set_ip_info(accessPointNetif, &ip), "Setting the setup network's address");
        esp_netif_set_dns_info(accessPointNetif, ESP_NETIF_DNS_MAIN, &dns);
        esp_netif_dhcps_option(accessPointNetif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &offer, sizeof(offer));
        esp_netif_dhcps_start(accessPointNetif);

        wifi_config_t config = {};
        strncpy((char *)config.ap.ssid, ssid, sizeof(config.ap.ssid));
        config.ap.ssid_len = strlen(ssid);
        strncpy((char *)config.ap.password, password, sizeof(config.ap.password));
        config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        // not TKIP too, it's insecure and iPhones call the network weak for it
        config.ap.pairwise_cipher = WIFI_CIPHER_TYPE_CCMP;
        config.ap.channel = 1;
        config.ap.max_connection = 4;

        accessPointOn = true;
        if (!startStation() || !check(esp_wifi_set_config(WIFI_IF_AP, &config), "Setting the setup network"))
        {
            stopAccessPoint();
            return false;
        }
        return true;
    }

    void stopAccessPoint()
    {
        if (accessPointOn)
        {
            accessPointOn = false;
            check(esp_wifi_set_mode(WIFI_MODE_STA), "Closing the setup network");
        }
    }

    esp_ip4_addr_t accessPointIp()
    {
        esp_netif_ip_info_t ip = {};
        if (accessPointNetif != nullptr)
        {
            esp_netif_get_ip_info(accessPointNetif, &ip);
        }
        return ip.ip;
    }

    void stop()
    {
        accessPointOn = false;
        if (running)
        {
            esp_wifi_stop();
            running = false;
        }
    }
}

#endif
