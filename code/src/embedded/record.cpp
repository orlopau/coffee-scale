#ifndef NATIVE

#include <Arduino.h>
#include <WiFi.h>
#include <esp_littlefs.h>
#include <sys/stat.h>

#include <string>

#include "embedded/record.h"
#include "display.h"
#include "interface.h"
#include "millis.h"
#include "recorder.h"
#include "recording_session.h"
#include "recording_store.h"
#include "ui/recorder_screens.h"

#define TAG "RECORDER"

#define FLASH_LABEL "spiffs"
#define FLASH_BASE "/littlefs"
#define RECORDINGS_DIR FLASH_BASE "/recordings"

#define CONNECT_TIMEOUT_MS 3000
#define RESPONSE_TIMEOUT_MS 5000

namespace Recording
{
    /** POSTs the file, true if the laptop answered 200. */
    static bool uploadFile(const std::string &path, const Server &server)
    {
        struct stat st;
        FILE *file = stat(path.c_str(), &st) == 0 ? fopen(path.c_str(), "rb") : nullptr;
        if (!file)
        {
            return false;
        }

        WiFiClient client;
        if (!client.connect(server.ip, server.port, CONNECT_TIMEOUT_MS))
        {
            ESP_LOGI(TAG, "Cannot connect to %s:%u", server.ip.toString().c_str(), server.port);
            fclose(file);
            return false;
        }

        client.printf("POST %s HTTP/1.1\r\n"
                      "Host: %s:%u\r\n"
                      "Content-Type: text/csv\r\n"
                      "Content-Length: %u\r\n"
                      "Connection: close\r\n\r\n",
                      server.path, server.ip.toString().c_str(), server.port, (unsigned int)st.st_size);

        uint8_t buffer[512];
        bool sent = true;
        size_t length;
        while (sent && (length = fread(buffer, 1, sizeof(buffer), file)) > 0)
        {
            sent = client.write(buffer, length) == length;
        }
        fclose(file);
        if (!sent)
        {
            ESP_LOGI(TAG, "Sending the recording failed");
            client.stop();
            return false;
        }

        unsigned long start = millis();
        while (!client.available() && client.connected() && millis() - start < RESPONSE_TIMEOUT_MS)
        {
            delay(10);
        }
        String status = client.readStringUntil('\n');
        client.stop();

        // e.g. "HTTP/1.0 200 OK"
        bool ok = status.startsWith("HTTP/1.") && status.substring(9, 12) == "200";
        ESP_LOGI(TAG, "Upload of %s: %s", path.c_str(), ok ? "saved" : status.c_str());
        return ok;
    }

    /** The recordings live in the partition labelled spiffs, as a LittleFS. */
    static bool mountFlash()
    {
        if (esp_littlefs_mounted(FLASH_LABEL))
        {
            return true;
        }
        esp_vfs_littlefs_conf_t conf = {};
        conf.base_path = FLASH_BASE;
        conf.partition_label = FLASH_LABEL;
        conf.format_if_mount_failed = true;
        esp_err_t err = esp_vfs_littlefs_register(&conf);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Mounting the flash failed: %s", esp_err_to_name(err));
        }
        return err == ESP_OK;
    }

    static void render(const RecordingSession &session)
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        RecorderScreens::recording(canvas, session.state());
        canvas.flush();
    }

    void run(SampleSource &samples, const Server *server, float gramsPerCount)
    {
        ESP_LOGI(TAG, "Recording %s", server ? "with a server" : "without a server");

        // The long press that found the server may still be held. Wait until it
        // is released, so it doesn't count as a press of the recorder.
        while (Interface::isEncoderPressed())
        {
            Interface::update();
            delay(10);
        }
        Interface::discardClicks();

        Recorder recorder;
        RecordingSession session(recorder, gramsPerCount, server != nullptr);
        RecordingStore store(RECORDINGS_DIR);
        // the first mount formats the partition, which takes some seconds
        bool storageReady = mountFlash() && store.begin();
        session.begin(storageReady ? store.count() : 0, storageReady);
        render(session);

        for (;;)
        {
            Interface::update();
            bool changed = false;

            Sample sample;
            while (samples.next(sample))
            {
                session.sample(sample.ms, sample.raw);
                changed = true;
            }

            if (Interface::getEncoderTurns() != 0)
            {
                session.tare(now());
                changed = true;
            }

            ClickType click = Interface::consumeEncoderClick();
            if (click == ClickType::SINGLE && session.click(now()) != RecordingSession::Click::Ignored)
            {
                Interface::buzzerTone(50);
                changed = true;
            }
            else if (click == ClickType::LONG)
            {
                switch (session.longPress())
                {
                case RecordingSession::Action::Save:
                    render(session);
                    session.saveFinished(store.save(recorder, FIRMWARE_VERSION, gramsPerCount));
                    changed = true;
                    break;
                case RecordingSession::Action::Send:
                {
                    bool ok = true;
                    for (std::string path = store.oldest(); ok && !path.empty(); path = store.oldest())
                    {
                        render(session);
                        ok = uploadFile(path, *server) && store.remove(path);
                        if (ok)
                        {
                            session.sent();
                        }
                    }
                    session.sendFinished(ok);
                    changed = true;
                    break;
                }
                case RecordingSession::Action::None:
                    break;
                }
            }

            if (click == ClickType::LONG)
            {
                // presses made while saving or sending must not act on the next screen
                Interface::discardClicks();
            }

            if (changed)
            {
                render(session);
            }
        }
    }
}

#endif
