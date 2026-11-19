#ifndef NATIVE

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "grinder/xenia.h"

/**
 * The scale as the Xenia the Mahlkönig grinder polls: a small HTTP server on
 * port 80 and the name coffee-scale.local, both only while the setting is on
 * and the scale is on its Wi-Fi. A background task keeps Wi-Fi and the
 * server up; the requests are answered from the server's own task.
 */
namespace GrinderServer
{
    /**
     * Starts the task, once, if the setting is on; otherwise setEnabled(true) starts it. The machine and the lock it is guarded by outlive
     * it. Call from the loop's task: it starts the Wi-Fi driver, whose first
     * start must be there.
     */
    void begin(Xenia::Machine &machine, SemaphoreHandle_t lock);
    /** Whether the grinder is served; off stops the server and the radio; the Wi-Fi driver stays initialised until the next restart. From the loop's task. */
    void setEnabled(bool on);
    /**
     * Stops the server and the radio (the driver stays initialised) until resume(), and returns when
     * that is done: the Wi-Fi setup needs port 80 too.
     */
    void pause();
    void resume();
    /** The station's IP address as text into out, empty if not connected. */
    void address(char *out, size_t size);
    /** now() of the grinder's last /api/v2/mako, 0 if there was none. */
    uint32_t lastPollMs();
}

#endif
