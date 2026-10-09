#include "mock/mock_display.h"

namespace Display
{
    void begin() {}
    Canvas &canvas() { return mockCanvas; }
    void drawOpener() {}
}
