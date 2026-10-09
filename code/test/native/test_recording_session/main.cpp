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

void test_records_samples_and_clicks(void)
{
    session->begin(0);
    session->sample(0, 1000);
    session->sample(100, 1400);

    TEST_ASSERT_EQUAL_FLOAT(4.0f, session->state().grams);
    TEST_ASSERT_EQUAL(Status::Recording, session->state().status);
    TEST_ASSERT_EQUAL(100, session->state().recordedMs);

    TEST_ASSERT_TRUE(session->click(150));
    TEST_ASSERT_EQUAL(1, session->state().clicks);
    TEST_ASSERT_EQUAL(2, recorder->sampleCount());
}

void test_long_press_uploads(void)
{
    session->begin(0);
    session->sample(0, 1000);

    TEST_ASSERT_TRUE(session->longPress());
    TEST_ASSERT_EQUAL(Status::Uploading, session->state().status);
    session->sample(100, 1100);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());

    session->uploadFinished(true, 500);
    TEST_ASSERT_EQUAL(Status::Recording, session->state().status);
    TEST_ASSERT_EQUAL(0, recorder->sampleCount());

    session->sample(500, 2000);
    TEST_ASSERT_EQUAL_FLOAT(0, session->state().grams);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
}

void test_failed_upload_keeps_recording(void)
{
    session->begin(0);
    session->sample(0, 1000);
    session->click(50);
    session->longPress();

    session->uploadFinished(false, 500);
    TEST_ASSERT_EQUAL(Status::UploadFailed, session->state().status);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    TEST_ASSERT_EQUAL(1, recorder->clickCount());

    session->sample(600, 1100);
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());

    TEST_ASSERT_TRUE(session->longPress());
    TEST_ASSERT_EQUAL(1, recorder->sampleCount());
    session->uploadFinished(true, 1000);
    TEST_ASSERT_EQUAL(0, recorder->sampleCount());
    TEST_ASSERT_EQUAL(0, recorder->clickCount());
}

void test_click_ignored_while_not_recording(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f);
    smallSession.begin(0);
    smallSession.sample(0, 1000);

    smallSession.longPress();
    TEST_ASSERT_FALSE(smallSession.click(10)); // uploading
    smallSession.uploadFinished(false, 20);
    TEST_ASSERT_FALSE(smallSession.click(30)); // upload failed
    TEST_ASSERT_EQUAL(0, small.clickCount());

    smallSession.longPress();
    smallSession.uploadFinished(true, 100);
    smallSession.sample(100, 1000);
    smallSession.sample(200, 1000);
    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
    TEST_ASSERT_FALSE(smallSession.click(250)); // full
    TEST_ASSERT_EQUAL(0, smallSession.state().clicks);
}

void test_full(void)
{
    Recorder small(2);
    RecordingSession smallSession(small, 0.01f);
    smallSession.begin(0);
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
    smallSession.begin(0);
    smallSession.sample(0, 1000);
    TEST_ASSERT_TRUE(smallSession.click(50));

    TEST_ASSERT_EQUAL(Status::Full, smallSession.state().status);
}

void test_weight_shown_while_not_recording(void)
{
    session->begin(0);
    session->sample(0, 1000);
    session->longPress();
    session->uploadFinished(false, 100);

    session->sample(200, 1500); // not recorded, but the scale still shows what is on it
    TEST_ASSERT_EQUAL_FLOAT(5.0f, session->state().grams);
}

void test_no_memory(void)
{
    Recorder none(0);
    RecordingSession noneSession(none, 0.01f);
    noneSession.begin(0);

    TEST_ASSERT_EQUAL(Status::NoMemory, noneSession.state().status);
    TEST_ASSERT_FALSE(noneSession.longPress());
    TEST_ASSERT_FALSE(noneSession.click(10));
    noneSession.sample(20, 1000);
    TEST_ASSERT_EQUAL(Status::NoMemory, noneSession.state().status);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_records_samples_and_clicks);
    RUN_TEST(test_long_press_uploads);
    RUN_TEST(test_failed_upload_keeps_recording);
    RUN_TEST(test_click_ignored_while_not_recording);
    RUN_TEST(test_full);
    RUN_TEST(test_full_by_click);
    RUN_TEST(test_weight_shown_while_not_recording);
    RUN_TEST(test_no_memory);
    return UNITY_END();
}
