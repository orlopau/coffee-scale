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

void ButtonTask::buzz(unsigned int count, unsigned int onMs, unsigned int offMs)
{
    // without the task, nothing would switch the buzzer off again
    if (task == nullptr || count == 0 || onMs == 0)
    {
        return;
    }
    // all at once, so the task never finishes the old pattern with the new one's times,
    // and the pin with it, so a late write can't switch the buzzer on after the task ended the pattern
    taskENTER_CRITICAL(&buzzMux);
    buzzStartMs = millis();
    buzzCount = count;
    buzzOnMs = onMs;
    buzzOffMs = offMs;
    buzzing = true;
    digitalWrite(PIN_BUZZER, HIGH);
    taskEXIT_CRITICAL(&buzzMux);
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

        bool switchBuzzer = false;
        bool buzzerOn = false;
        taskENTER_CRITICAL(&buzzMux);
        if (buzzing)
        {
            const uint32_t elapsed = millis() - buzzStartMs;
            const uint32_t period = buzzOnMs + buzzOffMs;
            if (elapsed >= period * (buzzCount - 1) + buzzOnMs)
            {
                buzzing = false;
            }
            else
            {
                buzzerOn = elapsed % period < buzzOnMs;
            }
            switchBuzzer = true;
        }
        taskEXIT_CRITICAL(&buzzMux);
        if (switchBuzzer)
        {
            digitalWrite(PIN_BUZZER, buzzerOn ? HIGH : LOW);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(PERIOD_MS));
    }
}

#endif
