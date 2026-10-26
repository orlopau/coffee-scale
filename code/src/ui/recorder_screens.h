#pragma once

#include <stdint.h>
#include "canvas.h"

/**
 * The screen of the load cell recorder, a hidden developer feature (see the
 * README). The recorder only runs on the device, its screen is drawn here so
 * native tests can render it too.
 */
namespace RecorderScreens
{
    enum class Status : uint8_t
    {
        Idle,
        Recording,
        Saving,
        SaveFailed,
        Sending,
        SendFailed,
        Full,
        NoMemory,
        NoStorage,
    };

    struct State
    {
        Status status;
        /** Weight since the recording started, only for orientation. */
        float grams;
        unsigned long recordedMs;
        unsigned long remainingMs;
        unsigned int clicks;
        /** Recordings saved in flash. */
        unsigned int saved;
        bool hasServer;
        /** While sending: the recording on its way, and how many there were when it started. */
        unsigned int sendingIndex;
        unsigned int sendingTotal;
    };

    void recording(Canvas &canvas, const State &state);
}
