#ifndef NATIVE

#include "embedded/loadcell_task.h"

#include <Arduino.h>
#include <driver/gpio.h>

#include "constants.h"
#include "loadcell.h"

#define TAG "LOADCELL"

// above the loop task (1), on the loop's core, so WiFi on the other core
// doesn't delay the reading
#define TASK_PRIORITY 5
#define TASK_CORE 1
#define TASK_STACK 3072

bool LoadCellTask::begin()
{
    LoadCell::begin();

    queue = xQueueCreate(QUEUE_LENGTH, sizeof(Sample));
    if (queue == nullptr)
    {
        ESP_LOGE(TAG, "Cannot create the queue");
        return false;
    }
    if (xTaskCreatePinnedToCore(run, "loadcell", TASK_STACK, this, TASK_PRIORITY, &task, TASK_CORE) != pdPASS)
    {
        ESP_LOGE(TAG, "Cannot start the task");
        return false;
    }

    // after the task exists, the interrupt notifies it
    attachInterruptArg(PIN_HX711_DAT, onDataReady, this, FALLING);
    return true;
}

bool LoadCellTask::next(Sample &sample) { return queue != nullptr && xQueueReceive(queue, &sample, 0) == pdTRUE; }

void ARDUINO_ISR_ATTR LoadCellTask::onDataReady(void *task)
{
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(static_cast<LoadCellTask *>(task)->task, &woken);
    if (woken)
    {
        portYIELD_FROM_ISR();
    }
}

void LoadCellTask::run(void *task) { static_cast<LoadCellTask *>(task)->loop(); }

void LoadCellTask::loop()
{
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(WAIT_MS));

        // The read's own clock pulses toggle the data pin and wake the task
        // again, and a wakeup can come from the timeout. Only a low data pin
        // means there is a reading.
        if (!LoadCell::isReady())
        {
            continue;
        }

        uint32_t ms = millis();
        gpio_intr_disable(static_cast<gpio_num_t>(PIN_HX711_DAT));
        int32_t raw = LoadCell::read();
        gpio_intr_enable(static_cast<gpio_num_t>(PIN_HX711_DAT));

        push({ms, raw});
    }
}

void LoadCellTask::push(const Sample &sample)
{
    if (xQueueSend(queue, &sample, 0) == pdTRUE)
    {
        if (droppedInARow > 0)
        {
            ESP_LOGW(TAG, "Queue was full, dropped the %u oldest readings", droppedInARow);
            droppedInARow = 0;
        }
        return;
    }

    // full: make room by dropping the oldest
    Sample oldest;
    xQueueReceive(queue, &oldest, 0);
    xQueueSend(queue, &sample, 0);
    dropped = dropped + 1;
    droppedInARow++;
}

#endif
