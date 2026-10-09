#pragma once
#include "canvas.h"

namespace Display
{
    void begin();

    /** Canvas that modes, the mode switcher and the updater draw on. */
    Canvas &canvas();

    /** Shows the boot splash screen (logo, name and firmware version). */
    void drawOpener();
};
