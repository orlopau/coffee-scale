#include <unity.h>

#include <algorithm>
#include <climits>

#include "mock/mock_canvas.h"
#include "shot_curve.h"
#include "ui/shot_graph.h"

/** Remembers the rows the graph drew on, for what the screenshots can't show: off-screen drawing. */
class RowCanvas : public RecordingCanvas
{
public:
    int top = INT_MAX;
    int bottom = INT_MIN;

    void drawPixel(int x, int y) override { row(y); }
    void drawLine(int x0, int y0, int x1, int y1) override
    {
        row(y0);
        row(y1);
    }

private:
    void row(int y)
    {
        top = std::min(top, y);
        bottom = std::max(bottom, y);
    }
};

void setUp(void) {}

void tearDown(void) {}

// e.g. the cup partly lifted at the click: the end is far below the shot's highest point
void test_points_above_the_yield_stay_on_screen(void)
{
    ShotCurve shot;
    for (uint32_t t = 100; t <= 9000; t += 100)
    {
        shot.add(t, 4.0f * t / 1000);
    }
    shot.add(9200, 20);
    shot.finish(9200);
    TEST_ASSERT_TRUE(shot.isFinished());

    RowCanvas canvas;
    ShotGraph::render(canvas, shot, nullptr);
    TEST_ASSERT_GREATER_OR_EQUAL(0, canvas.top);
    TEST_ASSERT_LESS_THAN(64, canvas.bottom);
}

// the same for the last shot behind it
void test_last_shot_points_above_both_yields_stay_on_screen(void)
{
    ShotCurve shot;
    for (uint32_t t = 100; t <= 6000; t += 100)
    {
        shot.add(t, 2.0f * t / 1000);
    }
    shot.finish(6000);
    ShotCurve last;
    for (uint32_t t = 100; t <= 9000; t += 100)
    {
        last.add(t, 4.0f * t / 1000);
    }
    last.add(9200, 10);
    last.finish(9200);

    RowCanvas canvas;
    ShotGraph::render(canvas, shot, &last);
    TEST_ASSERT_GREATER_OR_EQUAL(0, canvas.top);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_points_above_the_yield_stay_on_screen);
    RUN_TEST(test_last_shot_points_above_both_yields_stay_on_screen);
    return UNITY_END();
}
