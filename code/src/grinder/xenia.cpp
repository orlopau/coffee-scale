#include "grinder/xenia.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>

#include "logger.h"

#define TAG "XENIA"

namespace Xenia
{
    namespace
    {
        // signed, so the wrap of millis() does no harm
        int32_t since(uint32_t nowMs, uint32_t thenMs)
        {
            return static_cast<int32_t>(nowMs - thenMs);
        }

        // the path, without query string and trailing slash, is `expected`
        bool pathIs(const char *path, const char *expected)
        {
            size_t length = strcspn(path, "?");
            if (length > 1 && path[length - 1] == '/')
            {
                length--;
            }
            return length == strlen(expected) && strncmp(path, expected, length) == 0;
        }
    }

    const char *phaseName(Phase phase)
    {
        switch (phase)
        {
        case Phase::Ready:
            return "Ready";
        case Phase::Running:
            return "Running";
        case Phase::Finishing:
            return "Finishing";
        }
        return "?";
    }

    void Machine::start(uint32_t nowMs)
    {
        // a repeat in Finishing or Ready would start a shot nobody pulls, and tare the cup
        const bool repeat = everStarted && since(nowMs, startMs) < static_cast<int32_t>(START_REPEAT_MS);
        if (repeat || currentPhase == Phase::Running)
        {
            LOGW(TAG, "Start ignored: %s, %ld ms after the last start (%s)",
                 repeat ? "a repeat of the last knob press" : "already Running", (long)since(nowMs, startMs),
                 phaseName(currentPhase));
            return;
        }
        LOGI(TAG, "Start: %s -> Running", phaseName(currentPhase));
        everStarted = true;
        currentPhase = Phase::Running;
        startMs = nowMs;
        shotStarted = false;
        pending = Pending::None;
        seenInPhase = false;
    }

    void Machine::shotRuns()
    {
        if (currentPhase != Phase::Running)
        {
            LOGW(TAG, "Shot runs ignored: %s", phaseName(currentPhase));
            shotStarted = false;
            return;
        }
        LOGI(TAG, "Shot runs: no timeout any more");
        shotStarted = true;
    }

    void Machine::finish(uint32_t nowMs, uint32_t timeMs, float weightG)
    {
        if (currentPhase != Phase::Running || pending != Pending::None)
        {
            LOGW(TAG, "Finish (%lu ms, %.1f g) ignored: %s%s", (unsigned long)timeMs, (double)weightG,
                 phaseName(currentPhase), pending != Pending::None ? ", an end is pending already" : "");
            return;
        }
        LOGI(TAG, "Finish pending: %lu ms, %.1f g%s", (unsigned long)timeMs, (double)weightG,
             seenInPhase ? "" : ", until the grinder has seen Running");
        pending = Pending::Finish;
        pendingTimeMs = timeMs;
        pendingWeightG = std::isfinite(weightG) ? weightG : 0;
    }

    void Machine::abort(uint32_t nowMs)
    {
        if (currentPhase != Phase::Running || pending != Pending::None)
        {
            LOGW(TAG, "Abort ignored: %s%s", phaseName(currentPhase),
                 pending != Pending::None ? ", an end is pending already" : "");
            return;
        }
        LOGI(TAG, "Abort pending, %ld ms after the start%s", (long)since(nowMs, startMs),
             seenInPhase ? "" : ", until the grinder has seen Running");
        pending = Pending::Abort;
        pendingTimeMs = static_cast<uint32_t>(since(nowMs, startMs));
        pendingWeightG = 0;
    }

    void Machine::beginFinishing(uint32_t nowMs)
    {
        currentPhase = Phase::Finishing;
        wasAborted = pending == Pending::Abort;
        lastTimeMs = pendingTimeMs;
        lastWeightG = pendingWeightG;
        finishingStartMs = nowMs;
        pending = Pending::None;
        seenInPhase = false;
        LOGI(TAG, "Running -> Finishing: %s, %lu ms, %.1f g", wasAborted ? "aborted" : "done",
             (unsigned long)lastTimeMs, (double)lastWeightG);
    }

    void Machine::update(uint32_t nowMs)
    {
        if (currentPhase == Phase::Running)
        {
            if (pending == Pending::None && !shotStarted && since(nowMs, startMs) >= static_cast<int32_t>(SHOT_TIMEOUT_MS))
            {
                LOGI(TAG, "No shot within %lu s of the start", (unsigned long)(SHOT_TIMEOUT_MS / 1000));
                abort(nowMs);
            }
            // a shot that ends before the grinder saw it start would leave
            // the grinder waiting in "starting"
            if (pending != Pending::None && seenInPhase)
            {
                beginFinishing(nowMs);
            }
        }
        else if (currentPhase == Phase::Finishing)
        {
            int32_t age = since(nowMs, finishingStartMs);
            bool held = age >= static_cast<int32_t>(FINISHING_HOLD_MS) && seenInPhase;
            if (held || age >= static_cast<int32_t>(FINISHING_MAX_MS))
            {
                LOGI(TAG, "Finishing -> Ready after %ld ms, %s", (long)age,
                     held ? "the grinder has seen it" : "the grinder never polled in it");
                currentPhase = Phase::Ready;
                seenInPhase = false;
            }
        }
    }

    std::string Machine::mako(uint32_t nowMs)
    {
        int status = 1;
        int extraction = 0;
        uint32_t timeMs = lastTimeMs;
        float weightG = lastWeightG;
        if (currentPhase == Phase::Running)
        {
            status = 3;
            extraction = 1;
            timeMs = static_cast<uint32_t>(since(nowMs, startMs));
            weightG = 0;
        }
        else if (currentPhase == Phase::Finishing)
        {
            status = 4;
            extraction = wasAborted ? 2 : 1;
        }
        if (!seenInPhase && currentPhase != Phase::Ready)
        {
            LOGI(TAG, "The grinder sees %s", phaseName(currentPhase));
        }
        seenInPhase = true;

        char body[256];
        snprintf(body, sizeof(body),
                 "{\"MA_STATUS\":%d,\"MA_EXTRACTION_STATUS\":%d,\"REAL_EXTRACTION_TIME_MS\":%lu,"
                 "\"PU_SENS_FLOW_METER_VOLUME\":%.1f,\"BG_SET_TEMP\":93.0,\"BG_SENS_TEMP\":93.0,"
                 "\"TANK_LEVEL\":1,\"script\":\"\",\"TIMESTAMP\":\"\"}",
                 status, extraction, static_cast<unsigned long>(timeMs), static_cast<double>(weightG));
        return body;
    }

    std::string identityJson(const char *serial)
    {
        char body[256];
        snprintf(body, sizeof(body),
                 "{\"MA_TYPE\":1,\"MA_SUBTYPE\":0,\"MA_SN\":\"%s\",\"FW_VERSION_MAJOR\":4,"
                 "\"FW_VERSION_MINOR\":226,\"ESP_FW_MAJOR\":3,\"ESP_FW_MINOR\":87,"
                 "\"MA_MAX_AMPERE\":13,\"MA_FIX_WATER_SUPPLY\":0,\"BLE\":0,\"MA_DELAY\":-1}",
                 serial);
        return body;
    }

    Request classify(const char *method, const char *path)
    {
        bool get = strcasecmp(method, "GET") == 0;
        bool post = strcasecmp(method, "POST") == 0;
        if (get && pathIs(path, "/api/v2/mako"))
        {
            return Request::Mako;
        }
        if (get && pathIs(path, "/api/v2/machine"))
        {
            return Request::Identity;
        }
        if (post && pathIs(path, "/api/v2/brewratio"))
        {
            return Request::BrewRatio;
        }
        if (post && pathIs(path, "/api/v2/scripts/execute"))
        {
            return Request::StartBrew;
        }
        return Request::Other;
    }

    float brewWeightFrom(const char *body)
    {
        static const char KEY[] = "\"SYNC_BEVERAGE_WEIGHT\"";
        const char *key = strstr(body, KEY);
        if (key == nullptr)
        {
            return NAN;
        }
        const char *p = key + strlen(KEY);
        while (*p == ' ')
        {
            p++;
        }
        if (*p != ':')
        {
            return NAN;
        }
        p++;
        while (*p == ' ')
        {
            p++;
        }
        if (*p == '"')
        {
            p++;
        }
        char *end = nullptr;
        float grams = strtof(p, &end);
        if (end == p || !std::isfinite(grams) || grams <= 0)
        {
            return NAN;
        }
        return grams;
    }
}
