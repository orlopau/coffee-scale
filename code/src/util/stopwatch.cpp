#include "stopwatch.h"
#include "millis.h"

void Stopwatch::start()
{
    startTime = now();
    running = true;
}

void Stopwatch::stop()
{
    stopTime = now();
    running = false;
}

void Stopwatch::toggle()
{
    if (running)
    {
        stop();
    }
    else
    {
        start();
    }
}

void Stopwatch::reset()
{
    startTime = 0;
    stopTime = 0;
    running = false;
}

unsigned long Stopwatch::getTime()
{
    if (running)
    {
        return now() - startTime;
    }
    else
    {
        return stopTime - startTime;
    }
}

bool Stopwatch::isRunning()
{
    return running;
}