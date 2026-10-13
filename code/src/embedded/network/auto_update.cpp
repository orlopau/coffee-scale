#ifndef NATIVE

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "auto_update_policy.h"
#include "embedded/constants.h"
#include "firmware_version.h"
#include "settings.h"
#include "data/localization.h"
#include "embedded/blocking_screens.h"
#include "embedded/network/auto_update.h"
#include "embedded/network/firmware_check.h"
#include "embedded/network/release_check.h"
#include "embedded/network/wifi_station.h"
#include "ui/updater_screens.h"
#include "interface.h"
#include "embedded/update.h"
#include "embedded/network/firmware_download.h"

#define TAG "AUTO_UPDATE"

namespace AutoUpdate
{
    // like the updater
    static const uint32_t CONNECT_TIMEOUT_MS = 15000;
    // the TLS handshake runs on it
    static const uint32_t TASK_STACK = 8192;
    // below the button (2) and load cell (5) tasks
    static const UBaseType_t TASK_PRIORITY = 1;
    // with Wi-Fi; the loop, the load cell and the button tasks run on core 1
    static const BaseType_t TASK_CORE = 0;
    // a release's download address, RELEASE_URL with a version
    static const size_t URL_SIZE = 160;

    /** Asks for the latest release and stores what to offer. Nothing is stored if it fails. */
    static void check()
    {
        WifiStation::Online online(CONNECT_TIMEOUT_MS);
        if (!online.connected())
        {
            ESP_LOGI(TAG, "No network, checking at the next start");
            return;
        }
        char latest[FirmwareVersion::SIZE];
        if (!ReleaseCheck::latestVersion(LATEST_RELEASE_URL, latest, sizeof(latest)))
        {
            ESP_LOGW(TAG, "No version found, checking at the next start");
            return;
        }
        char rolledBack[FirmwareVersion::SIZE];
        Settings::getText(Settings::UPDATE_ROLLED_BACK_KEY, rolledBack, sizeof(rolledBack));
        const bool offer = AutoUpdatePolicy::worthOffering(latest, FIRMWARE_VERSION, rolledBack);
        ESP_LOGI(TAG, "Latest release %s, running %s%s", latest, FIRMWARE_VERSION,
                 offer ? ", asking at the next start" : "");
        Settings::setText(Settings::UPDATE_FOUND_KEY, offer ? latest : "");
        Settings::setByte(Settings::UPDATE_STARTS_KEY, 0);
    }

    static void run(void *)
    {
        // Wi-Fi goes off when it returns, unless someone else needs it
        check();
        ESP_LOGI(TAG, "Check done, %u bytes of stack left", (unsigned)uxTaskGetStackHighWaterMark(nullptr));
        vTaskDelete(nullptr);
    }

    void begin()
    {
        // nothing stored: never checked, so this start checks
        const uint8_t starts = AutoUpdatePolicy::countStart(
            Settings::getByte(Settings::UPDATE_STARTS_KEY, AutoUpdatePolicy::CHECK_EVERY_STARTS - 1));
        Settings::setByte(Settings::UPDATE_STARTS_KEY, starts);
        if (!AutoUpdatePolicy::isDue(starts, FirmwareCheck::onProbation()))
        {
            return;
        }
        // the driver only, the radio stays off until someone is online
        if (!WifiStation::begin())
        {
            return;
        }
        if (!WifiStation::hasSavedNetwork())
        {
            // looks again in 5 starts, and gives the driver's memory back meanwhile
            Settings::setByte(Settings::UPDATE_STARTS_KEY, 0);
            WifiStation::end();
            return;
        }
        ESP_LOGI(TAG, "Checking for a newer firmware");
        if (xTaskCreatePinnedToCore(run, "autoUpdate", TASK_STACK, nullptr, TASK_PRIORITY, nullptr, TASK_CORE) !=
            pdPASS)
        {
            ESP_LOGE(TAG, "Cannot start the task");
        }
    }


    /** Installs exactly version and restarts; on failure says so until a click and returns. */
    static void install(const char *version)
    {
        Updater::showMessage(UPDATER_UPDATING);
        char url[URL_SIZE];
        const int length = snprintf(url, sizeof(url), RELEASE_URL, version);
        const bool fits = length > 0 && (size_t)length < sizeof(url);
        if (!fits)
        {
            ESP_LOGE(TAG, "The address for %s is too long", version);
        }
        ESP_LOGI(TAG, "Installing %s", url);

        // before the download switches the firmware to boot, so a rollback of it is remembered, see askAtStart()
        Settings::setText(Settings::UPDATE_INSTALLING_KEY, version);
        FirmwareDownload::Result result = FirmwareDownload::Result::Failed;
        if (fits)
        {
            WifiStation::Online online(CONNECT_TIMEOUT_MS);
            if (online.connected())
            {
                result = FirmwareDownload::run(url, Updater::showProgress, ESP.getSketchMD5().c_str());
            }
        } // Wi-Fi goes off
        if (result == FirmwareDownload::Result::Installed)
        {
            FirmwareCheck::expectNewFirmware();
            Updater::showMessage(UPDATER_SUCCESS);
            delay(2000);
            esp_restart();
        }
        Settings::setText(Settings::UPDATE_INSTALLING_KEY, "");
        ESP_LOGW(TAG, "Installing %s failed, asking again at the next start", version);
        Updater::showMessage(UPDATER_FAILED);
        BlockingScreens::waitForClick();
    }

    void askAtStart(bool rolledBack)
    {
        char installing[FirmwareVersion::SIZE];
        Settings::getText(Settings::UPDATE_INSTALLING_KEY, installing, sizeof(installing));
        if (rolledBack && installing[0] != '\0')
        {
            ESP_LOGW(TAG, "%s was rolled back, not offering it again", installing);
            Settings::setText(Settings::UPDATE_ROLLED_BACK_KEY, installing);
            Settings::setText(Settings::UPDATE_INSTALLING_KEY, "");
        }

        char found[FirmwareVersion::SIZE];
        char rolled[FirmwareVersion::SIZE];
        Settings::getText(Settings::UPDATE_FOUND_KEY, found, sizeof(found));
        Settings::getText(Settings::UPDATE_ROLLED_BACK_KEY, rolled, sizeof(rolled));
        if (!AutoUpdatePolicy::shouldAsk(found, FIRMWARE_VERSION, rolled, FirmwareCheck::onProbation()))
        {
            return;
        }
        // index 0 is "Yes"
        const uint8_t choice =
            BlockingScreens::choose(2, [&found](Canvas &canvas, uint8_t index) { UpdaterScreens::ask(canvas, found, index); });
        if (choice == 0)
        {
            install(found);
        }
        else
        {
            ESP_LOGI(TAG, "Later, asking again at the next start");
        }
    }

    void clearInstalling()
    {
        char installing[FirmwareVersion::SIZE];
        Settings::getText(Settings::UPDATE_INSTALLING_KEY, installing, sizeof(installing));
        // most starts have nothing to clear, and needn't write
        if (installing[0] != '\0')
        {
            Settings::setText(Settings::UPDATE_INSTALLING_KEY, "");
        }
    }
}

#endif
