#ifndef NATIVE

#pragma once

#include "weighing/sample.h"

namespace Updater
{
    /**
     * Updates the firmware, or records the load cell for a dev server.
     *
     * @param scale the calibration in grams per count
     * @param samples the load cell's readings, for recordings
     */
    void update_firmware(float scale, SampleSource &samples);
}

#endif