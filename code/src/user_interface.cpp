#include <math.h>
#include <stdio.h>

char *formatWeight(float weight)
{
    static char buffer[10];
    sprintf(buffer, "%.2fg", weight);
    return buffer;
}

char *formatTime(unsigned long time)
{
    unsigned int secs = floor(time / 1000);
    unsigned int mins = floor(secs / 60);
    unsigned int millis = time % 1000;

    static char buffer[10];
    sprintf(buffer, "%d:%d.%d", mins, secs, millis / 100);
    return buffer;
}


