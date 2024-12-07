#ifndef NATIVE

#include <AutoConnectCore.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WebServer.h>
#include <WiFi.h>

#include "constants.h"
#include "display.h"
#include "interface.h"
#include "data/localization.h"

#define TAG "UPDATER"

namespace Updater
{
    AutoConnect portal;

    void started() { ESP_LOGI(TAG, "CALLBACK:  HTTP update process started"); }

    void finished() { ESP_LOGI(TAG, "CALLBACK:  HTTP update process finished"); }

    void progress(Display &display, int cur, int total)
    {
        ESP_LOGI(TAG, "CALLBACK:  HTTP update process at %d of %d bytes...", cur, total);
        static char progress[32];
        sprintf(progress, UPDATER_PROGRESS, ((float)cur / (float)total) * 100);
        display.centerText(progress, 13);
    }

    void error(int err) { ESP_LOGI(TAG, "CALLBACK:  HTTP update fatal error code %d\n", err); }

    bool onCaptivePortalStart(Display &display, IPAddress &address)
    {
        ESP_LOGI(TAG, "Captive portal started");
        static char text[64];
        sprintf(text, UPDATER_WIFI_CONNECT_MANUAL, WiFi.softAPSSID().c_str());
        display.text(text);
        return true;
    }

    void update_firmware(Display &display)
    {
        display.centerText(UPDATER_UPDATING, 13);
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

        auto captivePortalStartFunc = std::bind(onCaptivePortalStart, std::ref(display), std::placeholders::_1);
        portal.onDetect(captivePortalStartFunc);

        if (portal.begin())
        {
            ESP_LOGI(TAG, "WiFi connected");
            display.centerText(UPDATER_WIFI_CONNECTED, 13);
        }

        delay(1000);

        // choose firmware type
        int selectedQualifier = 0;
        const uint8_t numQualifiers = 2;
        const char *names[] = {"Deutsch", "English"};
        const char *qualifiers[] = {"_de", "_en"};
        while (Interface::getEncoderClick() != ClickType::SINGLE)
        {
            selectedQualifier += static_cast<int>(Interface::getEncoderDirection());
            if (selectedQualifier < 0)
            {
                selectedQualifier = 0;
            }
            else if (selectedQualifier >= numQualifiers)
            {
                selectedQualifier = numQualifiers - 1;
            }
            display.switcher("Choose Language", selectedQualifier, numQualifiers, names);
            Interface::update();
        }

        HTTPUpdate httpUpdate;
        httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        httpUpdate.onStart(started);
        httpUpdate.onEnd(finished);
        auto progressFunc = std::bind(progress, std::ref(display), std::placeholders::_1, std::placeholders::_2);
        httpUpdate.onProgress(progressFunc);
        httpUpdate.onError(error);
        
        WiFiClientSecure client;
        client.setInsecure();

        char url[128];
        snprintf(url, 128, UPDATE_URL, qualifiers[selectedQualifier]);
        ESP_LOGI(TAG, "Update URL: %s", url);
        t_httpUpdate_return code = httpUpdate.update(client, url);

        switch (code)
        {
        case HTTP_UPDATE_FAILED:
            ESP_LOGI(TAG, "HTTP_UPDATE_FAILED Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            display.centerText(UPDATER_FAILED, 13);
            break;

        case HTTP_UPDATE_NO_UPDATES:
            ESP_LOGI(TAG, "HTTP_UPDATE_NO_UPDATES");
            display.centerText(UPDATER_NO_UPDATE, 13);
            break;

        case HTTP_UPDATE_OK:
            ESP_LOGI(TAG, "HTTP_UPDATE_OK");
            display.centerText(UPDATER_SUCCESS, 13);
            break;
        }

        for (;;)
        {
            delay(1000);
        }
    }
}

#endif