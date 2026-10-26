#include <stdio.h>

#include "ui/recorder_screens.h"
#include "ui/updater_screens.h"
#include "data/localization.h"
#include "formatters.h"

namespace RecorderScreens
{
    static void statusText(char *buffer, size_t size, const State &state)
    {
        const char *text = "";
        switch (state.status)
        {
        case Status::Idle:
            text = RECORDER_IDLE;
            break;
        case Status::Recording:
            text = RECORDER_RECORDING;
            break;
        case Status::Saving:
            text = RECORDER_SAVING;
            break;
        case Status::SaveFailed:
            text = RECORDER_SAVE_FAILED;
            break;
        case Status::Sending:
            snprintf(buffer, size, RECORDER_SENDING, state.sendingIndex, state.sendingTotal);
            return;
        case Status::SendFailed:
            text = RECORDER_SEND_FAILED;
            break;
        case Status::Full:
            text = RECORDER_FULL;
            break;
        case Status::NoMemory:
            text = RECORDER_NO_MEMORY;
            break;
        case Status::NoStorage:
            text = RECORDER_NO_STORAGE;
            break;
        }
        snprintf(buffer, size, "%s", text);
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
        if (state.status == Status::NoStorage)
        {
            UpdaterScreens::message(canvas, RECORDER_NO_STORAGE);
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

        bool showSaved = state.status == Status::Idle || state.status == Status::SendFailed || state.status == Status::Sending;
        snprintf(text, sizeof(text), showSaved ? RECORDER_SAVED : RECORDER_MARKERS, showSaved ? state.saved : state.clicks);
        canvas.drawText(0, 48, text);
        if (showSaved && !state.hasServer)
        {
            canvas.drawText(canvas.width() - canvas.textWidth(RECORDER_NO_SERVER), 48, RECORDER_NO_SERVER);
        }

        canvas.drawHLine(0, 51, canvas.width());
        statusText(text, sizeof(text), state);
        canvas.drawText(0, 61, text);
    }
}
