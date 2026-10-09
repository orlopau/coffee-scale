#ifndef NATIVE

#include <Arduino.h>
#include <WiFi.h>

#include "embedded/record.h"
#include "display.h"
#include "interface.h"
#include "loadcell.h"
#include "millis.h"
#include "recorder.h"
#include "recording_session.h"
#include "ui/recorder_screens.h"

#define TAG "RECORDER"

#define CONNECT_TIMEOUT_MS 3000
#define RESPONSE_TIMEOUT_MS 5000

namespace Recording
{
    /** Collects small writes into packets, a TCP write per CSV line would be slow. */
    class BufferedWriter
    {
    public:
        explicit BufferedWriter(WiFiClient &client) : client(client) {}

        void write(const char *data, size_t length)
        {
            while (length > 0)
            {
                size_t chunk = min(length, sizeof(buffer) - used);
                memcpy(buffer + used, data, chunk);
                used += chunk;
                data += chunk;
                length -= chunk;
                if (used == sizeof(buffer))
                {
                    flush();
                }
            }
        }

        void flush()
        {
            if (used > 0 && client.write(buffer, used) != used)
            {
                failed = true;
            }
            used = 0;
        }

        bool hasFailed() const { return failed; }

    private:
        WiFiClient &client;
        uint8_t buffer[512];
        size_t used = 0;
        bool failed = false;
    };

    static bool upload(const Recorder &recorder, IPAddress server, uint16_t port, const char *path, float gramsPerCount)
    {
        WiFiClient client;
        if (!client.connect(server, port, CONNECT_TIMEOUT_MS))
        {
            ESP_LOGI(TAG, "Cannot connect to %s:%u", server.toString().c_str(), port);
            return false;
        }

        size_t length = recorder.writeCsv(FIRMWARE_VERSION, gramsPerCount, [](const char *, size_t) {});
        client.printf("POST %s HTTP/1.1\r\n"
                      "Host: %s:%u\r\n"
                      "Content-Type: text/csv\r\n"
                      "Content-Length: %u\r\n"
                      "Connection: close\r\n\r\n",
                      path, server.toString().c_str(), port, (unsigned int)length);

        BufferedWriter writer(client);
        recorder.writeCsv(FIRMWARE_VERSION, gramsPerCount,
                          [&](const char *data, size_t length) { writer.write(data, length); });
        writer.flush();
        if (writer.hasFailed())
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
        ESP_LOGI(TAG, "Upload of %u bytes: %s", (unsigned int)length, ok ? "saved" : status.c_str());
        return ok;
    }

    static void render(const RecordingSession &session)
    {
        Canvas &canvas = Display::canvas();
        canvas.clear();
        RecorderScreens::recording(canvas, session.state());
        canvas.flush();
    }

    void run(IPAddress server, uint16_t port, const char *path, float gramsPerCount)
    {
        ESP_LOGI(TAG, "Recording for http://%s:%u%s", server.toString().c_str(), port, path);

        // The long press that found the server may still be held. Wait until it
        // is released, so it doesn't count as the long press that uploads.
        while (Interface::isEncoderPressed())
        {
            Interface::update();
            delay(10);
        }
        Interface::consumeEncoderClick();

        Recorder recorder;
        RecordingSession session(recorder, gramsPerCount);
        session.begin(now());
        render(session);

        for (;;)
        {
            Interface::update();
            bool changed = false;

            if (LoadCell::isReady())
            {
                session.sample(now(), LoadCell::read());
                changed = true;
            }

            ClickType click = Interface::consumeEncoderClick();
            if (click == ClickType::SINGLE && session.click(now()))
            {
                Interface::buzzerTone(50);
                changed = true;
            }
            else if (click == ClickType::LONG && session.longPress())
            {
                render(session);
                session.uploadFinished(upload(session.recorder(), server, port, path, gramsPerCount), now());
                changed = true;
            }

            if (changed)
            {
                render(session);
            }
        }
    }
}

#endif
