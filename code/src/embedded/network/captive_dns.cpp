#ifndef NATIVE

#include <errno.h>
#include <string.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <unistd.h>

#include "embedded/network/captive_dns.h"
#include "network/dns_answer.h"

#define TAG "DNS"

namespace CaptiveDns
{
    static const size_t PACKET = 512;

    static volatile bool running = false;
    static bool taskStarted = false;
    static SemaphoreHandle_t stopped = nullptr;
    static uint8_t address[4];

    static void serve(void *)
    {
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        sockaddr_in local = {};
        local.sin_family = AF_INET;
        local.sin_port = htons(53);
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        // wakes up now and then to see whether to stop
        timeval timeout = {0, 200000};
        if (sock < 0 || bind(sock, (sockaddr *)&local, sizeof(local)) != 0 ||
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
        {
            ESP_LOGE(TAG, "Opening port 53 failed: %d", errno);
            running = false;
        }

        static uint8_t query[PACKET];
        static uint8_t reply[PACKET];
        while (running)
        {
            sockaddr_in from = {};
            socklen_t fromLength = sizeof(from);
            const int length = recvfrom(sock, query, sizeof(query), 0, (sockaddr *)&from, &fromLength);
            if (length <= 0)
            {
                continue;
            }
            const size_t replyLength = DnsAnswer::reply(query, length, address, reply, sizeof(reply));
            if (replyLength > 0)
            {
                sendto(sock, reply, replyLength, 0, (sockaddr *)&from, fromLength);
            }
        }
        if (sock >= 0)
        {
            close(sock);
        }
        xSemaphoreGive(stopped);
        vTaskDelete(nullptr);
    }

    bool start(esp_ip4_addr_t ip)
    {
        if (taskStarted)
        {
            return true;
        }
        // esp_ip4_addr_t holds the address in network order, its first byte first
        memcpy(address, &ip.addr, sizeof(address));
        if (stopped == nullptr)
        {
            stopped = xSemaphoreCreateBinary();
        }
        running = true;
        if (stopped == nullptr || xTaskCreate(serve, "captive_dns", 3072, nullptr, 5, nullptr) != pdPASS)
        {
            ESP_LOGE(TAG, "Starting the task failed");
            running = false;
            return false;
        }
        taskStarted = true;
        return true;
    }

    void stop()
    {
        if (!taskStarted)
        {
            return;
        }
        running = false;
        xSemaphoreTake(stopped, pdMS_TO_TICKS(1000));
        taskStarted = false;
    }
}

#endif
