#pragma once

#include <cmath>
#include <cstdint>
#include <string>

// What the Mahlkönig E64 WS grinder sees when it polls the scale: a Xenia
// espresso machine. Pure logic, no I/O and no clock of its own, every call
// takes the time in milliseconds.
namespace Xenia
{
    // "finishing" is shown at least this long, and until the grinder has polled
    // once in it, but never longer than FINISHING_MAX_MS
    constexpr uint32_t FINISHING_HOLD_MS = 6000;
    constexpr uint32_t FINISHING_MAX_MS = 60000;
    // no shot after the knob press: the machine gives up; a shot that
    // started runs as long as it takes
    constexpr uint32_t SHOT_TIMEOUT_MS = 180000;
    // the E64 WS sends every knob press three times, 8.2-8.8 s apart, also
    // when it already sees Running: a knob press this soon after the one that
    // started the last shot is a repeat of it
    constexpr uint32_t START_REPEAT_MS = 20000;

    enum class Phase
    {
        Ready,
        Running,
        Finishing
    };

    class Machine
    {
    public:
        // the knob press; from Ready, and from Finishing, which it ends: the
        // grinder moved on from the last shot. A repeat of the last knob press
        // (START_REPEAT_MS) changes nothing, whatever the phase.
        void start(uint32_t nowMs);
        // the shot started, the timeout no longer applies; only while Running
        void shotRuns();
        // the shot ended; only while Running
        void finish(uint32_t nowMs, uint32_t timeMs, float weightG);
        // the shot did not happen; only while Running
        void abort(uint32_t nowMs);
        // the timeout, the end of finishing, and a pending finish or abort
        // once the grinder has seen Running
        void update(uint32_t nowMs);

        Phase phase() const { return currentPhase; }
        // of the current or the last Finishing
        bool aborted() const { return wasAborted; }

        // the body of /api/v2/mako; marks that the grinder saw this phase
        std::string mako(uint32_t nowMs);

        // the Brew Weight the grinder sent, NAN until set
        void setBrewWeight(float grams) { brewGrams = grams; }
        float brewWeight() const { return brewGrams; }

    private:
        enum class Pending
        {
            None,
            Finish,
            Abort
        };

        void beginFinishing(uint32_t nowMs);

        Phase currentPhase = Phase::Ready;
        // the knob press that started the last shot; repeats don't move it
        uint32_t startMs = 0;
        bool everStarted = false;
        bool shotStarted = false;
        Pending pending = Pending::None;
        uint32_t pendingTimeMs = 0;
        float pendingWeightG = 0;
        // the grinder polled since the phase began
        bool seenInPhase = false;
        uint32_t lastTimeMs = 0;
        float lastWeightG = 0;
        uint32_t finishingStartMs = 0;
        bool wasAborted = false;
        float brewGrams = NAN;
    };

    // for the log
    const char *phaseName(Phase phase);

    // the body of /api/v2/machine
    std::string identityJson(const char *serial);

    enum class Request
    {
        Identity,
        Mako,
        BrewRatio,
        StartBrew,
        Other
    };

    // a trailing "/" and a query string are ignored, the method is compared
    // without regard to case
    Request classify(const char *method, const char *path);

    // SYNC_BEVERAGE_WEIGHT from a brewratio body, as a number or a quoted
    // number; NAN if missing, unreadable, not finite or not above 0
    float brewWeightFrom(const char *body);
}
