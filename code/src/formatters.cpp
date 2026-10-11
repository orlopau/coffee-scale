#include <math.h>
#include <stdio.h>

#include "formatters.h"

char *formatWeight(float grams, int decimals)
{
    if (roundf(grams * powf(10, decimals)) == 0)
    {
        grams = 0;
    }
    static char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.*fg", decimals, grams);
    return buffer;
}

char *formatTime(unsigned long millis)
{
    unsigned int secs = floor(millis / 1000);
    unsigned int mins = floor(secs / 60);
    unsigned int _millis = millis % 1000;

    static char buffer[10];
    sprintf(buffer, "%02d:%02d.%d", mins, secs % 60, _millis / 100);
    return buffer;
}
