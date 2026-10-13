#ifndef NATIVE

#pragma once

#include <stddef.h>

#include "weighing/sample.h"

namespace Updater
{
    /** A short status message on the display, e.g. "Updating...". */
    void showMessage(const char *text);

    /** A download's progress on the display, as FirmwareDownload::Progress; total is 0 if unknown. */
    void showProgress(size_t done, size_t total);

    /**
     * Updates the firmware, or records the load cell for a dev server.
     *
     * @param scale the calibration in grams per count
     * @param samples the load cell's readings, for recordings
     */
    void update_firmware(float scale, SampleSource &samples);
}

#endif