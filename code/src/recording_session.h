#pragma once

#include "recorder.h"
#include "ui/recorder_screens.h"

/**
 * What the recorder does with the scale's inputs: a click starts a recording,
 * samples and further clicks go into the Recorder while it records, a turn
 * tares the shown weight, and a long press saves the recording. A long press
 * while idle sends the saved recordings to the laptop, if there is one. The
 * device only passes the inputs in and does the saving and sending, so this
 * logic runs in native tests.
 */
class RecordingSession
{
public:
    RecordingSession(Recorder &recorder, float gramsPerCount, bool hasServer);

    enum class Click
    {
        Ignored,
        Started,
        Marked,
    };

    enum class Action
    {
        None,
        Save,
        Send,
    };

    /**
     * Waits for the click that starts recording, or shows that there is no
     * memory or no storage for it. saved is the number of recordings in flash.
     */
    void begin(size_t saved, bool storageReady);
    /**
     * Recorded only while recording. The first sample of a recording, or of
     * waiting for one, is the tare of the shown weight.
     */
    void sample(unsigned long ms, long raw);
    /** Starts a recording at ms while waiting, places a marker while recording. The caller beeps unless ignored. */
    Click click(unsigned long ms);
    /** Tares the shown weight, and records the tare while recording. */
    void tare(unsigned long ms);
    /** Returns what the caller must do now: save the recording, send the saved ones, or nothing. */
    Action longPress();
    /** On success it waits for the next recording, otherwise the recording is kept for a retry. */
    void saveFinished(bool ok);
    /** One saved recording arrived and was deleted. */
    void sent();
    /** On success it waits for the next recording, otherwise the rest is kept for a retry. */
    void sendFinished(bool ok);

    RecorderScreens::State state() const;
    const Recorder &recorder() const { return rec; }

private:
    void becomeIdle();
    void startRecording(unsigned long ms);
    void checkFull();

    Recorder &rec;
    float gramsPerCount;
    bool hasServer;
    unsigned int saved = 0;
    unsigned int sendingIndex = 0;
    unsigned int sendingTotal = 0;
    RecorderScreens::Status status = RecorderScreens::Status::Idle;
    bool tared = false;
    long tareRaw = 0;
    long lastRaw = 0;
    bool hasSample = false;
};
