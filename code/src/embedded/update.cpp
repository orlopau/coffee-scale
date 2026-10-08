#ifndef NATIVE

#include <AutoConnectCore.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "constants.h"
#include "display.h"
#include "interface.h"
#include "data/localization.h"

#define TAG "UPDATER"

// mDNS service announced by scripts/serve_firmware.py (`pio run -t serve`)
#define DEV_SERVER_SERVICE "coffeescale-fw"
// each attempt waits up to 3 s for answers
#define DEV_SERVER_SEARCH_ATTEMPTS 3

namespace Updater
{
    AutoConnect portal;

    static void updateFromDevServer();
    static void runUpdate(WiFiClient &client, const char *url);

    void started() { ESP_LOGI(TAG, "CALLBACK:  HTTP update process started"); }

    void finished() { ESP_LOGI(TAG, "CALLBACK:  HTTP update process finished"); }

    void progress(int cur, int total)
    {
        ESP_LOGI(TAG, "CALLBACK:  HTTP update process at %d of %d bytes...", cur, total);
        static char progress[32];
        sprintf(progress, UPDATER_PROGRESS, ((float)cur / (float)total) * 100);
        Display::centerText(progress, 13);
    }

    void error(int err) { ESP_LOGI(TAG, "CALLBACK:  HTTP update fatal error code %d\n", err); }

    bool onCaptivePortalStart(IPAddress &address)
    {
        ESP_LOGI(TAG, "Captive portal started");
        static char text[64];
        sprintf(text, UPDATER_WIFI_CONNECT_MANUAL, WiFi.softAPSSID().c_str());
        Display::text(text);
        return true;
    }

    void update_firmware()
    {
        Display::centerText(UPDATER_UPDATING, 13);
        ESP_LOGI(TAG, "Updating firmware...");

        uint32_t id = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }

        char ssid[25];
        sprintf(ssid, "CoffeeScale-%d", id);
        AutoConnectConfig config(ssid, "");
        config.title = "CoffeeScale";
        config.boundaryOffset = 1024;
        portal.config(config);

        portal.onDetect(onCaptivePortalStart);

        if (portal.begin())
        {
            ESP_LOGI(TAG, "WiFi connected");
            Display::centerText(UPDATER_WIFI_CONNECTED, 13);
        }

        delay(1000);

        // The button may still be held from entering the updater at boot. Wait
        // until it is released, so holding it does not count as a long press.
        while (Interface::isEncoderPressed())
        {
            Interface::update();
            delay(10);
        }
        Interface::consumeEncoderClick();

        // choose firmware type
        int selectedQualifier = 0;
        const uint8_t numQualifiers = 2;
        const char *names[] = {"Deutsch", "English"};
        const char *qualifiers[] = {"_de", "_en"};
        for (;;)
        {
            ClickType click = Interface::getEncoderClick();
            if (click == ClickType::SINGLE)
            {
                break;
            }
            // hidden: long press updates from a developer's computer instead of GitHub
            if (click == ClickType::LONG)
            {
                Interface::consumeEncoderClick();
                updateFromDevServer(); // only returns if no server was found
            }

            selectedQualifier += static_cast<int>(Interface::getEncoderDirection());
            if (selectedQualifier < 0)
            {
                selectedQualifier = 0;
            }
            else if (selectedQualifier >= numQualifiers)
            {
                selectedQualifier = numQualifiers - 1;
            }
            Display::switcher("Choose Language", selectedQualifier, numQualifiers, names);
            Interface::update();
        }

        WiFiClientSecure client;
        client.setInsecure();

        char url[128];
        snprintf(url, 128, UPDATE_URL, qualifiers[selectedQualifier]);
        runUpdate(client, url);
    }

    static bool findDevServer(char *url, size_t urlSize)
    {
        if (!MDNS.begin("coffee-scale"))
        {
            ESP_LOGE(TAG, "mDNS could not be started");
            return false;
        }

        for (int attempt = 0; attempt < DEV_SERVER_SEARCH_ATTEMPTS; attempt++)
        {
            int found = MDNS.queryService(DEV_SERVER_SERVICE, "tcp");
            if (found > 0)
            {
                String path = MDNS.txt(0, "path");
                if (path.length() == 0)
                {
                    path = "/firmware.bin";
                }
                snprintf(url, urlSize, "http://%s:%u%s", MDNS.IP(0).toString().c_str(), MDNS.port(0), path.c_str());
                return true;
            }
        }
        return false;
    }

    static void updateFromDevServer()
    {
        Display::centerText(UPDATER_DEV_SEARCHING, 13);
        ESP_LOGI(TAG, "Searching for dev firmware server...");

        char url[128];
        if (!findDevServer(url, sizeof(url)))
        {
            ESP_LOGI(TAG, "No dev firmware server found");
            Display::centerText(UPDATER_DEV_NOT_FOUND, 13);
            delay(2000);
            return;
        }

        WiFiClient client;
        runUpdate(client, url);
    }

    static void runUpdate(WiFiClient &client, const char *url)
    {
        HTTPUpdate httpUpdate;
        httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        httpUpdate.onStart(started);
        httpUpdate.onEnd(finished);
        httpUpdate.onProgress(progress);
        httpUpdate.onError(error);

        ESP_LOGI(TAG, "Update URL: %s", url);
        t_httpUpdate_return code = httpUpdate.update(client, url);

        switch (code)
        {
        case HTTP_UPDATE_FAILED:
            ESP_LOGI(TAG, "HTTP_UPDATE_FAILED Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            Display::centerText(UPDATER_FAILED, 13);
            break;

        case HTTP_UPDATE_NO_UPDATES:
            ESP_LOGI(TAG, "HTTP_UPDATE_NO_UPDATES");
            Display::centerText(UPDATER_NO_UPDATE, 13);
            break;

        case HTTP_UPDATE_OK:
            ESP_LOGI(TAG, "HTTP_UPDATE_OK");
            Display::centerText(UPDATER_SUCCESS, 13);
            break;
        }

        for (;;)
        {
            delay(1000);
        }
    }
}

#endif