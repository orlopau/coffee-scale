#ifndef NATIVE

#pragma once

namespace Updater
{
    /** Updates the firmware, or records the load cell for a dev server. scale is the calibration in grams per count. */
    void update_firmware(float scale);
}

#endif