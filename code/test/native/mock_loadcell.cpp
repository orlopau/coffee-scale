#include "mock/mock_loadcell.h"

namespace LoadCell
{
    bool ready = true;
    long value = 0L;

    void begin() {}

    bool isReady() { return ready; }

    long read() { return value; }
}
