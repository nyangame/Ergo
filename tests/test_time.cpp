#include "test_framework.hpp"
#include "engine/core/time.hpp"

TEST_CASE(Time_Reset) {
    Time t;
    t.tick(0.1f);
    t.reset();
    ASSERT_NEAR(t.delta_time, 0.0f, 0.001f);
    ASSERT_NEAR(t.total_time, 0.0f, 0.001f);
    ASSERT_EQ(t.frame_count, (uint64_t)0);
}

TEST_CASE(Time_Tick) {
    Time t;
    t.reset();
    t.tick(1.0f / 60.0f);
    ASSERT_NEAR(t.delta_time, 1.0f / 60.0f, 0.0001f);
    ASSERT_EQ(t.frame_count, (uint64_t)1);
    ASSERT_TRUE(t.total_time > 0.0f);
}

TEST_CASE(Time_TimeScale) {
    Time t;
    t.reset();
    t.time_scale = 0.5f;
    t.tick(1.0f / 60.0f);
    ASSERT_NEAR(t.delta_time, (1.0f / 60.0f) * 0.5f, 0.0001f);
    ASSERT_NEAR(t.unscaled_delta_time, 1.0f / 60.0f, 0.0001f);
}

TEST_CASE(Time_TimeScale_Paused) {
    Time t;
    t.reset();
    t.time_scale = 0.0f;
    t.tick(1.0f / 60.0f);
    ASSERT_NEAR(t.delta_time, 0.0f, 0.0001f);
    ASSERT_NEAR(t.unscaled_delta_time, 1.0f / 60.0f, 0.0001f);
}

TEST_CASE(Time_FrameCount) {
    Time t;
    t.reset();
    for (int i = 0; i < 10; ++i) {
        t.tick(1.0f / 60.0f);
    }
    ASSERT_EQ(t.frame_count, (uint64_t)10);
}

TEST_CASE(Time_TotalTime) {
    Time t;
    t.reset();
    t.time_scale = 1.0f;
    for (int i = 0; i < 60; ++i) {
        t.tick(1.0f / 60.0f);
    }
    ASSERT_NEAR(t.total_time, 1.0f, 0.01f);
}
