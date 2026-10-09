#include "millis.h"

#ifdef NATIVE
// Starts well above zero, like a device that has been running for a while.
static unsigned long virtualTime = 1000000;

unsigned long now()
{
    return virtualTime;
}

void sleep_for(unsigned long millis)
{
    virtualTime += millis;
}

void set_now(unsigned long millis)
{
    virtualTime = millis;
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
