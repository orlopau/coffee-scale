#ifndef NATIVE

#pragma once

#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "button.h"
#include "embedded/click_queue.h"

/**
 * Reads the encoder's button every 5 ms in its own task, so a click is
 * detected even while the loop is blocked, and puts every finished click into
 * the queue. It also switches the buzzer on and off for a beep or a pattern of beeps.
 */
class ButtonTask
{
public:
    static constexpr uint32_t PERIOD_MS = 5;

    explicit ButtonTask(ClickQueue &clicks) : clicks(clicks) {}

    /** Starts the task. False, and logged, if it could not be started. */
    bool begin();
    /** The debounced state of the button. */
    bool isPressed() const { return pressed; }
    /** Beeps count times, each onMs long with offMs between, replacing a beep still running. Does nothing without the task. */
    void buzz(unsigned int count, unsigned int onMs, unsigned int offMs);

private:
    static void run(void *task);
    void loop();

    ClickQueue &clicks;
    Button button;
    TaskHandle_t task = nullptr;
    std::atomic<bool> pressed{false};
    /** Guards the pattern: buzz() and the task's loop run in different tasks. */
    portMUX_TYPE buzzMux = portMUX_INITIALIZER_UNLOCKED;
    bool buzzing = false;
    uint32_t buzzStartMs = 0;
    uint32_t buzzCount = 0;
    uint32_t buzzOnMs = 0;
    uint32_t buzzOffMs = 0;
};

#endif
