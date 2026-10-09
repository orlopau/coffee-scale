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
        Recording,
        Uploading,
        UploadFailed,
        Full,
        NoMemory,
    };

    struct State
    {
        Status status;
        /** Weight since the recording started, only for orientation. */
        float grams;
        unsigned long recordedMs;
        unsigned long remainingMs;
        unsigned int clicks;
    };

    void recording(Canvas &canvas, const State &state);
}
