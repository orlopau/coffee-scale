#pragma once

#ifdef NATIVE
#include <chrono>
unsigned long now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
#else
#include <Arduino.h>
unsigned long now()
{
    return millis();
}
#endif