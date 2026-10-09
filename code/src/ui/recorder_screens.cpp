#include <stdio.h>

#include "ui/recorder_screens.h"
#include "ui/updater_screens.h"
#include "data/localization.h"
#include "formatters.h"

namespace RecorderScreens
{
    static const char *statusText(Status status)
    {
        switch (status)
        {
        case Status::Recording:
            return RECORDER_RECORDING;
        case Status::Uploading:
            return RECORDER_UPLOADING;
        case Status::UploadFailed:
            return RECORDER_UPLOAD_FAILED;
        case Status::Full:
            return RECORDER_FULL;
        case Status::NoMemory:
            return RECORDER_NO_MEMORY;
        }
        return "";
    }

    /** m:ss, formatTime() with its tenths is too wide here. */
    static void formatDuration(char *buffer, size_t size, const char *prefix, unsigned long ms)
    {
        unsigned long seconds = ms / 1000;
        snprintf(buffer, size, "%s%lu:%02lu", prefix, seconds / 60, seconds % 60);
    }

    void recording(Canvas &canvas, const State &state)
    {
        if (state.status == Status::NoMemory)
        {
            UpdaterScreens::message(canvas, RECORDER_NO_MEMORY);
            return;
        }

        canvas.setFont(Font::Number22);
        canvas.drawText(0, 22, formatWeight(state.grams));

        char text[24];
        canvas.setFont(Font::Small);
        formatDuration(text, sizeof(text), "", state.recordedMs);
        canvas.drawText(0, 36, text);
        formatDuration(text, sizeof(text), "-", state.remainingMs);
        canvas.drawText(canvas.width() - canvas.textWidth(text), 36, text);

        snprintf(text, sizeof(text), RECORDER_MARKERS, state.clicks);
        canvas.drawText(0, 48, text);

        canvas.drawHLine(0, 51, canvas.width());
        canvas.drawText(0, 61, statusText(state.status));
    }
}
