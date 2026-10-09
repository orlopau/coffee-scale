#ifdef NATIVE

#include <cmath>

#include "native/simulated_scale.h"
#include "millis.h"

static const double SAMPLE_INTERVAL_MS = 1000.0 / SimulatedScale::SAMPLES_PER_SECOND;

void SimulatedScale::setWeight(float grams) { weight = grams < 0 ? 0 : grams; }

void SimulatedScale::addWeight(float grams) { setWeight(weight + grams); }

float SimulatedScale::getFlow() const { return shotRunning ? shotFlow(now() - shotStartTime) : flow; }

void SimulatedScale::setFlow(float gramsPerSecond)
{
    flow = gramsPerSecond;
    shotRunning = false;
}

void SimulatedScale::startShot()
{
    update();
    flow = 0;
    shotRunning = true;
    shotStartTime = now();
}

float SimulatedScale::shotFlow(unsigned long shotTime) const
{
    const float MAX_FLOW = 1.6f;
    const float t = shotTime / 1000.0f;
    if (t < 6)
    {
        return 0; // pre-infusion, nothing in the cup yet
    }
    if (t < 9)
    {
        return MAX_FLOW * (t - 6) / 3;
    }
    if (t < 30)
    {
        return MAX_FLOW;
    }
    if (t < SHOT_DURATION_MS / 1000.0f)
    {
        return MAX_FLOW * (SHOT_DURATION_MS / 1000.0f - t) / 3; // the pump stopped, the last drops
    }
    return 0;
}

void SimulatedScale::update()
{
    const unsigned long time = now();
    if (!started)
    {
        started = true;
        lastUpdateTime = time;
        nextSampleTime = time;
    }

    // integrated in steps of 1 ms, so the result hardly depends on how often update() is called
    for (unsigned long t = lastUpdateTime; t < time; t++)
    {
        if (shotRunning)
        {
            if (t - shotStartTime >= SHOT_DURATION_MS)
            {
                shotRunning = false;
                break;
            }
            weight += shotFlow(t - shotStartTime) / 1000.0f;
        }
        else
        {
            weight += flow / 1000.0f;
        }
    }
    if (weight < 0)
    {
        weight = 0;
    }
    lastUpdateTime = time;
}

bool SimulatedScale::isReady()
{
    update();
    return now() >= nextSampleTime;
}

long SimulatedScale::read()
{
    update();
    nextSampleTime += SAMPLE_INTERVAL_MS;
    // after a pause, e.g. a slow loop, the HX711 does not catch up but just measures again
    if (nextSampleTime < now())
    {
        nextSampleTime = now() + SAMPLE_INTERVAL_MS;
    }

    float counts = EMPTY_COUNTS + weight * COUNTS_PER_GRAM;
    if (noise)
    {
        counts += noiseDistribution(random);
    }
    return lround(counts);
}

#endif
