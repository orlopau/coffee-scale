#ifndef NATIVE

#include "embedded/click_queue.h"

#include <Arduino.h>

#define TAG "CLICKS"

bool ClickQueue::begin()
{
    queue = xQueueCreate(LENGTH, sizeof(ClickEvent));
    if (queue == nullptr)
    {
        ESP_LOGE(TAG, "Cannot create the queue");
        return false;
    }
    return true;
}

void ClickQueue::push(const ClickEvent &event)
{
    if (queue == nullptr)
    {
        return;
    }

    if (xQueueSend(queue, &event, 0) == pdTRUE)
    {
        if (droppedInARow > 0)
        {
            ESP_LOGW(TAG, "Queue was full, dropped the %u oldest clicks", droppedInARow);
            droppedInARow = 0;
        }
        return;
    }

    // full: make room by dropping the oldest
    ClickEvent oldest;
    xQueueReceive(queue, &oldest, 0);
    xQueueSend(queue, &event, 0);
    droppedInARow++;
}

bool ClickQueue::next(ClickEvent &event) { return queue != nullptr && xQueueReceive(queue, &event, 0) == pdTRUE; }

#endif
