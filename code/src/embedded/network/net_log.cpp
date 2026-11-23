#if !defined(NATIVE) && defined(NET_LOG)

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>
#include <esp_log.h>
#include <lwip/sockets.h>

#include <atomic>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "net_log_text.h"
#include "embedded/network/dev_server_search.h"
#include "embedded/network/net_log.h"
#include "embedded/network/wifi_station.h"

#define TAG "NET_LOG"

namespace NetLog
{
    // lines wait here for the sender; about 80 lines of 100 bytes
    static const size_t RING_BUFFER_SIZE = 8192;
    // like the updater
    static const uint32_t CONNECT_TIMEOUT_MS = 15000;
    // one mDNS query
    static const uint32_t SEARCH_TIMEOUT_MS = 3000;
    // between connecting or searching attempts
    static const uint32_t RETRY_MS = 5000;
    // how long the sender waits for a line before it checks the connection
    static const uint32_t RECEIVE_WAIT_MS = 500;
    static const uint32_t TASK_STACK = 4096;
    // below the button (2) and load cell (5) tasks
    static const UBaseType_t TASK_PRIORITY = 1;
    // room for the "lines dropped" notice on the stack, with its NUL
    static const size_t NOTICE_SIZE = 48;

    static RingbufHandle_t buffer = nullptr;
    static vprintf_like_t previous = nullptr;
    // lines there was no room for, reported in the buffer by capture()
    static std::atomic<uint32_t> dropped{0};

    /**
     * Copies one formatted line into the ring buffer. Runs in whichever task
     * logs, so it must not block or take much stack: it formats straight into
     * the ring buffer instead of a buffer of its own.
     */
    static void capture(const char *format, va_list args)
    {
        // an ISR can't take the ring buffer's lock
        if (xPortInIsrContext() || buffer == nullptr)
        {
            return;
        }
        va_list forLength;
        va_copy(forLength, args);
        const int length = vsnprintf(nullptr, 0, format, forLength);
        va_end(forLength);
        if (length <= 0)
        {
            return;
        }

        // Lines lost earlier are reported here, ahead of this line, so the
        // gap shows where it happened. Taking the count with exchange() lets
        // only one logger write the notice. Like a line, the item is the text
        // and its NUL, exactly: the sender sends what is in it.
        const uint32_t lost = dropped.exchange(0);
        if (lost != 0)
        {
            char text[NOTICE_SIZE];
            const size_t noticeLength = NetLogText::droppedLine(lost, text, sizeof(text));
            void *notice = nullptr;
            if (xRingbufferSendAcquire(buffer, &notice, noticeLength + 1, 0) != pdTRUE)
            {
                // the lines lost so far, and this one
                dropped.fetch_add(lost + 1);
                return;
            }
            memcpy(notice, text, noticeLength + 1);
            xRingbufferSendComplete(buffer, notice);
        }

        void *item = nullptr;
        if (xRingbufferSendAcquire(buffer, &item, length + 1, 0) != pdTRUE)
        {
            dropped++;
            return;
        }
        vsnprintf(static_cast<char *>(item), length + 1, format, args);
        xRingbufferSendComplete(buffer, item);
    }

    /** For IDF's own log lines, which go through esp_log_write. */
    static int hook(const char *format, va_list args)
    {
        va_list forUart;
        va_copy(forUart, args);
        const int result = previous ? previous(format, forUart) : 0;
        va_end(forUart);
        capture(format, args);
        return result;
    }

    // how often the sender logs the stack its tasks have left
    static const uint32_t STACK_LOG_MS = 60000;

    /** Logs the smallest free stack (in bytes on the ESP32) of the sender and of the button and load cell tasks. */
    static void logStack()
    {
        char text[80];
        size_t used = snprintf(text, sizeof(text), "net_log %u", (unsigned)uxTaskGetStackHighWaterMark(nullptr));
        const char *names[] = {"button", "loadcell"};
        for (const char *name : names)
        {
            TaskHandle_t task = xTaskGetHandle(name);
            if (task != nullptr && used < sizeof(text))
            {
                used += snprintf(text + used, sizeof(text) - used, ", %s %u", name, (unsigned)uxTaskGetStackHighWaterMark(task));
            }
        }
        ESP_LOGI(TAG, "Stack left (bytes): %s", text);
    }

    /** Sends the ring buffer's lines until sending fails or the Wi-Fi is gone. */
    static void sendLines(int sock, const sockaddr_in &destination)
    {
        const sockaddr *to = reinterpret_cast<const sockaddr *>(&destination);
        uint32_t lastStackLog = millis();
        while (WifiStation::connected())
        {
            if (millis() - lastStackLog >= STACK_LOG_MS)
            {
                lastStackLog = millis();
                logStack();
            }
            size_t size = 0;
            char *item = static_cast<char *>(xRingbufferReceive(buffer, &size, pdMS_TO_TICKS(RECEIVE_WAIT_MS)));
            if (item == nullptr)
            {
                continue;
            }
            // without the NUL the capture added
            const size_t length = NetLogText::stripColors(item, size - 1);
            const int sent = sendto(sock, item, length, 0, to, sizeof(destination));
            vRingbufferReturnItem(buffer, item);
            if (sent < 0)
            {
                // Wi-Fi TX buffers full during a burst: drop this line, give
                // them time and go on; capture() reports it as dropped
                if (errno == ENOMEM && WifiStation::connected())
                {
                    dropped.fetch_add(1);
                    vTaskDelay(pdMS_TO_TICKS(10));
                    continue;
                }
                return;
            }
        }
    }

    static void run(void *)
    {
        for (;;)
        {
            // dropped with the scope: a new attempt reconnects
            WifiStation::Online online(CONNECT_TIMEOUT_MS);
            if (!online.connected())
            {
                vTaskDelay(pdMS_TO_TICKS(RETRY_MS));
                continue;
            }
            DevServerSearch::Server server{};
            bool found = false;
            while (WifiStation::connected())
            {
                found = DevServerSearch::find(server, SEARCH_TIMEOUT_MS) && server.logPort != 0;
                if (found)
                {
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(RETRY_MS));
            }
            if (!found)
            {
                continue;
            }
            const int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock < 0)
            {
                vTaskDelay(pdMS_TO_TICKS(RETRY_MS));
                continue;
            }
            sockaddr_in destination{};
            destination.sin_family = AF_INET;
            destination.sin_addr.s_addr = server.ip.addr;
            destination.sin_port = htons(server.logPort);
            char address[16];
            inet_ntoa_r(destination.sin_addr, address, sizeof(address));
            ESP_LOGI(TAG, "Sending the log to %s:%u", address, (unsigned)server.logPort);
            sendLines(sock, destination);
            close(sock);
        }
    }

    void begin()
    {
        buffer = xRingbufferCreate(RING_BUFFER_SIZE, RINGBUF_TYPE_NOSPLIT);
        if (buffer == nullptr)
        {
            return;
        }
        // IDF's own lines; the app's go through __wrap_log_printfv below
        previous = esp_log_set_vprintf(hook);
        // Held for the whole run and never destroyed: AutoUpdate calls
        // WifiStation::end() when there is no saved network, which would shut
        // the driver down under mDNS while the sender waits between retries.
        // With this Radio held, end() does nothing in the log firmware.
        static WifiStation::Radio radio;
        if (!WifiStation::begin())
        {
            return;
        }
        xTaskCreate(run, "net_log", TASK_STACK, nullptr, TASK_PRIORITY, nullptr);
    }
}

/**
 * The app's ESP_LOGx are Arduino's log_x, which call log_printf and then
 * log_printfv and ets_printf, never esp_log_write, so the hook above doesn't
 * see them. The framework already wraps log_printf itself (esp_diagnostics),
 * and that wrapper ends in log_printfv, so this wraps log_printfv instead:
 * the linker flag --wrap=log_printfv (esp32dev-log only) sends its calls here.
 */
extern "C" int __real_log_printfv(const char *format, va_list args);
extern "C" int __wrap_log_printfv(const char *format, va_list args)
{
    va_list forBuffer;
    va_copy(forBuffer, args);
    const int result = __real_log_printfv(format, args);
    NetLog::capture(format, forBuffer);
    va_end(forBuffer);
    return result;
}

#endif
