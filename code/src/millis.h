#pragma once

#ifdef NATIVE
#include <cstdio>
#define PRINTF printf
#define PRINTLN println
#else
#include <Arduino.h>
#define PRINTF Serial.printf
#define PRINTLN Serial.println
#endif

unsigned long now();
void sleep_for(unsigned long millis);