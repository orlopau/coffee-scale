#pragma once

#include "display.h"
#include "mock/mock_canvas.h"

namespace Display
{
    /** What Display::canvas() returns in native tests. */
    inline RecordingCanvas mockCanvas;

    inline void reset() { mockCanvas.reset(); }
}
