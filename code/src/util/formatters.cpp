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
    sprintf(buffer, "%02d:%02d.%d", mins, secs % 60, _millis / 100);
    return buffer;
}
