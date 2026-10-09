#pragma once

#include "native/simulated_scale.h"
#include "native/virtual_display.h"

/**
 * The hardware of the emulator, which the window controls: the firmware
 * reads it through the same Interface, LoadCell, Display, Battery and
 * Settings functions as on the device.
 */
namespace Emulator
{
    /** What the load cell measures. */
    extern SimulatedScale scale;

    /** Turns the encoder by a number of detents, positive is clockwise. */
    void turnEncoder(int detents);
    /** Presses or releases the encoder's button. */
    void setButtonPressed(bool pressed);
    bool isButtonPressed();
    /** True while the buzzer sounds. */
    bool isBuzzing();

    VirtualDisplay &display();
}
