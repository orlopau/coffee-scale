#pragma once

#ifdef NATIVE
#include <chrono>
#include <thread>
inline unsigned long now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline void sleep_for(unsigned long millis)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}
#else
#include <Arduino.h>
inline unsigned long now()
{
    return millis();
}

inline void sleep_for(unsigned long millis)
{
    delay(millis);
}
#endif