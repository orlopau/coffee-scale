#pragma once

#ifdef NATIVE
#include <cstdio>
#include <cstdarg>
inline void log_default(const char *tag, const char *format, ...)
{
    printf(tag);
    printf(": ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

#define LOGI log_default
#else
#include <Arduino.h>
#define LOGI ESP_LOGI
#endif