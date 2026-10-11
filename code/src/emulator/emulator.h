#pragma once

#include "native/simulated_scale.h"
#include "native/virtual_display.h"
#include "weighing/sample.h"

/**
 * The hardware of the emulator, which the window controls: the firmware
 * reads it through the same Interface, Display, Battery and Settings
 * functions as on the device, and its load cell through a SampleSource.
 */
namespace Emulator
{
    /** What the load cell measures. */
    extern SimulatedScale scale;
    /** The load cell's readings, timestamped when they become ready, like the device's load cell task. */
    SampleSource &samples();

    /** Turns the encoder by a number of detents, positive is clockwise. */
    void turnEncoder(int detents);
    /** Presses or releases the encoder's button. */
    void setButtonPressed(bool pressed);
    bool isButtonPressed();
    /** True while the buzzer sounds. */
    bool isBuzzing();

    VirtualDisplay &display();
}
