#include "millis.h"

#ifdef NATIVE
#include <chrono>
#include <thread>
#include <cstdio>
unsigned long now()
{
    auto now = std::chrono::system_clock::now();
    auto time_since_epoch = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch);
    auto count = millis.count();
    return count;
    // return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void sleep_for(unsigned long millis)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}
#else
#include <Arduino.h>
unsigned long now()
{
    return millis();
}

void sleep_for(unsigned long millis)
{
    delay(millis);
}
#endif