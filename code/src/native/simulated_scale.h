#pragma once

#include <random>

/**
 * The load cell with whatever is on the scale, simulated: turns the weight in
 * grams into raw HX711 readings, at the HX711's sample rate. Weight can be
 * put on and taken off, liquid can flow into the cup at a constant rate, and
 * an espresso shot follows a typical flow profile.
 *
 * Time comes from now(), so it follows the virtual clock of native builds.
 */
class SimulatedScale
{
public:
    /** Raw counts per gram, roughly what the scale's load cell gives. */
    static constexpr float COUNTS_PER_GRAM = 420;
    /** Raw reading of the empty scale, a load cell never reads zero. */
    static constexpr long EMPTY_COUNTS = 84000;
    /** The HX711 measures 10 or 80 times per second, depending on its RATE pin. */
    static constexpr float SAMPLES_PER_SECOND = 80;
    /** Standard deviation of the noise, when enabled. */
    static constexpr float NOISE_COUNTS = 25;
    /** How long an espresso shot started with startShot() runs, in ms. */
    static constexpr unsigned long SHOT_DURATION_MS = 33000;

    /** Grams on the scale, without noise. */
    float getWeight() const { return weight; }
    void setWeight(float grams);
    /** Puts weight on the scale, or takes it off if negative. The scale never holds less than nothing. */
    void addWeight(float grams);

    /** Grams per second flowing onto the scale, also during a shot. */
    float getFlow() const;
    /** Lets liquid flow onto the scale at a constant rate, until set to 0. Ends a running shot. */
    void setFlow(float gramsPerSecond);

    /**
     * Starts an espresso shot: the first drops come after 6 s of
     * pre-infusion, the flow rises to 1.6 g/s, and stops when the pump stops
     * at 30 s. About 38 g end up in the cup, a little more than the espresso
     * mode's default target.
     */
    void startShot();
    bool isShotRunning() const { return shotRunning; }

    void setNoise(bool enabled) { noise = enabled; }
    bool hasNoise() const { return noise; }

    /** Advances the simulation to now(). Call regularly, e.g. once per loop. */
    void update();
    /** True if a new reading is available, like the HX711's data ready signal. */
    bool isReady();
    /** The current reading. The next one is ready one sample interval later. */
    long read();

private:
    float weight = 0;
    float flow = 0;
    bool shotRunning = false;
    unsigned long shotStartTime = 0;
    bool noise = false;

    bool started = false;
    unsigned long lastUpdateTime = 0;
    double nextSampleTime = 0;

    std::mt19937 random{1};
    std::normal_distribution<float> noiseDistribution{0, NOISE_COUNTS};

    float shotFlow(unsigned long shotTime) const;
};
