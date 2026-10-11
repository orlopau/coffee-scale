#ifndef NATIVE

#include "embedded/button_task.h"

#include <Arduino.h>

#include "constants.h"

#define TAG "BUTTON"

// above the loop task (1), below the load cell task (5), on the loop's core
#define TASK_PRIORITY 2
#define TASK_CORE 1
// room for ESP_LOGW when the click queue overflows, like the load cell task
#define TASK_STACK 3072

bool ButtonTask::begin()
{
    if (xTaskCreatePinnedToCore(run, "button", TASK_STACK, this, TASK_PRIORITY, &task, TASK_CORE) != pdPASS)
    {
        task = nullptr;
        ESP_LOGE(TAG, "Cannot start the task");
        return false;
    }
    return true;
}

void ButtonTask::buzz(unsigned int durationMs)
{
    // without the task, nothing would switch the buzzer off again
    if (task == nullptr)
    {
        return;
    }
    // the end first, so the task never sees the buzzer on with an old end
    buzzEndMs = millis() + durationMs;
    buzzing = true;
    digitalWrite(PIN_BUZZER, HIGH);
}

void ButtonTask::run(void *task) { static_cast<ButtonTask *>(task)->loop(); }

void ButtonTask::loop()
{
    TickType_t lastWake = xTaskGetTickCount();
    for (;;)
    {
        button.update(digitalRead(PIN_ENC_BTN) == LOW);
        pressed = button.isPressed();

        // Button reports each click in exactly one update
        ClickType type = button.getClickType();
        if (type != ClickType::NONE)
        {
            clicks.push({millis(), type});
        }

        if (buzzing && static_cast<int32_t>(millis() - buzzEndMs) >= 0)
        {
            digitalWrite(PIN_BUZZER, LOW);
            buzzing = false;
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(PERIOD_MS));
    }
}

#endif
