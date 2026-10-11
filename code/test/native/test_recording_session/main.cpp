#include <unity.h>

#include "recorder.h"
#include "recording_session.h"

using Status = RecorderScreens::Status;

static Recorder *recorder;
static RecordingSession *session;

void setUp(void)
{
    recorder = new Recorder();
    session = new RecordingSession(*recorder, 0.01f);
}

void tearDown(void)
{
    delete session;
    delete recorder;
}

/** Begins and starts recording at ms 0. */
static void startRecording(RecordingSession &s)
{
    s.begin();
    TEST_ASSERT_EQUAL(RecordingSession::Click::Started, s.click(0));
}

void test_waits_for_click_to_start(void)
{
    session->begin();
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
    session->sample(0, 1000);
    session->sample(100, 1300);
    TEST_ASSERT_EQUAL(0, recorder->sampleCount());
    TEST_ASSERT_FALSE(session->longPress());

    TEST_ASSERT_EQUAL(RecordingSession::Click::Started, session->click(200));
    TEST_ASSERT_EQUAL(Status::Recording, session->state().status);
    TEST_ASSERT_EQUAL(0, recorder->clickCount());
    session->sample(200, 1500);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
}

void test_weight_shown_while_idle(void)
{
    session->begin();
    session->sample(0, 1000);
    session->sample(100, 1300);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, session->state().grams);
}

void test_records_samples_and_clicks(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->sample(100, 1400);

    TEST_ASSERT_EQUAL_FLOAT(4.0f, session->state().grams);
    TEST_ASSERT_EQUAL(Status::Recording, session->state().status);
    TEST_ASSERT_EQUAL(100, session->state().recordedMs);

    TEST_ASSERT_EQUAL(RecordingSession::Click::Marked, session->click(150));
    TEST_ASSERT_EQUAL(1, session->state().clicks);
    TEST_ASSERT_EQUAL(2, recorder->sampleCount());
}

void test_long_press_uploads_and_waits(void)
{
    startRecording(*session);
    session->sample(0, 1000);

    TEST_ASSERT_TRUE(session->longPress());
    TEST_ASSERT_EQUAL(Status::Uploading, session->state().status);
    session->sample(100, 1100);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());

    session->uploadFinished(true);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
    session->sample(500, 2000);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount()); // the uploaded one, not recording
    TEST_ASSERT_FALSE(session->longPress());

    TEST_ASSERT_EQUAL(RecordingSession::Click::Started, session->click(600));
    TEST_ASSERT_EQUAL(0, recorder->sampleCount());
    session->sample(600, 2500);
    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
}

void test_failed_upload_keeps_recording(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->click(50);
    session->longPress();

    session->uploadFinished(false);
    TEST_ASSERT_EQUAL(Status::UploadFailed, session->state().status);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    TEST_ASSERT_EQUAL(1, recorder->clickCount());

    session->sample(600, 1100);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());

    TEST_ASSERT_TRUE(session->longPress());
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    session->uploadFinished(true);
    TEST_ASSERT_EQUAL(Status::Idle, session->state().status);
}

void test_click_ignored_while_not_recording(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f);
    startRecording(smallSession);
    smallSession.sample(0, 1000);

    smallSession.longPress();
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, smallSession.click(10)); // uploading
    smallSession.uploadFinished(false);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, smallSession.click(30)); // upload failed
    TEST_ASSERT_EQUAL(0, small.clickCount());

    smallSession.longPress();
    smallSession.uploadFinished(true);
    smallSession.click(100);
    smallSession.sample(100, 1000);
    smallSession.sample(200, 1000);
    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, smallSession.click(250)); // full
    TEST_ASSERT_EQUAL(0, smallSession.state().clicks);
}

void test_full(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f);
    startRecording(smallSession);
    smallSession.sample(0, 1000);
    smallSession.sample(100, 1010);

    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
    smallSession.sample(200, 1020);
    TEST_ASSERT_EQUAL(2, small.sampleCount());
    TEST_ASSERT_TRUE(smallSession.longPress());
}

void test_full_by_click(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f);
    startRecording(smallSession);
    smallSession.sample(0, 1000);
    TEST_ASSERT_EQUAL(RecordingSession::Click::Marked, smallSession.click(50));

    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
}

void test_weight_shown_while_not_recording(void)
{
    startRecording(*session);
    session->sample(0, 1000);
    session->longPress();
    session->uploadFinished(false);

    session->sample(200, 1500); // not recorded, but the scale still shows what is on it
    TEST_ASSERT_EQUAL_FLOAT(5.0f, session->state().grams);
}

void test_no_memory(void)
{
    Recorder none(0);
    RecordingSession noneSession(none, 0.01f);
    noneSession.begin();

    TEST_ASSERT_EQUAL(Status::NoMemory, noneSession.state().status);
    TEST_ASSERT_FALSE(noneSession.longPress());
    TEST_ASSERT_EQUAL(RecordingSession::Click::Ignored, noneSession.click(10));
    noneSession.sample(20, 1000);
    TEST_ASSERT_EQUAL(Status::NoMemory, noneSession.state().status);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_waits_for_click_to_start);
    RUN_TEST(test_weight_shown_while_idle);
    RUN_TEST(test_records_samples_and_clicks);
    RUN_TEST(test_long_press_uploads_and_waits);
    RUN_TEST(test_failed_upload_keeps_recording);
    RUN_TEST(test_click_ignored_while_not_recording);
    RUN_TEST(test_full);
    RUN_TEST(test_full_by_click);
    RUN_TEST(test_weight_shown_while_not_recording);
    RUN_TEST(test_no_memory);
    return UNITY_END();
}
