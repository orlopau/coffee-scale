#include "stopwatch.h"
#include "millis.h"

void Stopwatch::start() { startAt(now()); }

void Stopwatch::startAt(unsigned long startMs)
{
    startTime = startMs;
    running = true;
}

void Stopwatch::stop() { stopAt(now()); }

void Stopwatch::stopAt(unsigned long stopMs)
{
    // stopped already, the time stays
    if (!running)
    {
        return;
    }
    // a stop before the start counts as no time
    stopTime = (long)(stopMs - startTime) < 0 ? startTime : stopMs;
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