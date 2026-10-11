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
 * the queue. It also switches the buzzer off when a beep is over.
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
    /** Switches the buzzer on, the task switches it off after the duration. Does nothing without the task. */
    void buzz(unsigned int durationMs);

private:
    static void run(void *task);
    void loop();

    ClickQueue &clicks;
    Button button;
    TaskHandle_t task = nullptr;
    std::atomic<bool> pressed{false};
    std::atomic<bool> buzzing{false};
    std::atomic<uint32_t> buzzEndMs{0};
};

#endif
