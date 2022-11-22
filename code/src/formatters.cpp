#include <math.h>
#include <stdio.h>

char *formatWeight(float grams)
{
    static char buffer[10];
    sprintf(buffer, "%.2fg", grams);
    return buffer;
}

char *formatTime(unsigned long millis)
{
    unsigned int secs = floor(millis / 1000);
    unsigned int mins = floor(secs / 60);
    unsigned int _millis = millis % 1000;

    static char buffer[10];
    sprintf(buffer, "%d:%d.%d", mins, secs, _millis / 100);
    return buffer;
}
