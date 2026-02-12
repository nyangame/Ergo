#include "test_framework.hpp"
#include "engine/debug/profiler.hpp"

TEST_CASE(Profiler_BeginEnd) {
    Profiler profiler;
    profiler.begin("test");
    // Do some trivial work
    volatile int x = 0;
    for (int i = 0; i < 1000; ++i) x += i;
    profiler.end();

    float ms = profiler.get("test");
    ASSERT_TRUE(ms >= 0.0f);
}

TEST_CASE(Profiler_Results) {
    Profiler profiler;
    profiler.begin("section_a");
    profiler.end();
    profiler.begin("section_b");
    profiler.end();

    auto& results = profiler.results();
    ASSERT_EQ(results.size(), (size_t)2);
    ASSERT_TRUE(results.count("section_a") > 0);
    ASSERT_TRUE(results.count("section_b") > 0);
}

TEST_CASE(Profiler_Clear) {
    Profiler profiler;
    profiler.begin("test");
    profiler.end();
    profiler.clear();
    ASSERT_EQ(profiler.results().size(), (size_t)0);
}

TEST_CASE(Profiler_GetNonexistent) {
    Profiler profiler;
    float ms = profiler.get("nonexistent");
    ASSERT_NEAR(ms, 0.0f, 0.001f);
}

TEST_CASE(Profiler_Nested) {
    Profiler profiler;
    profiler.begin("outer");
    profiler.begin("inner");
    profiler.end();
    profiler.end();

    ASSERT_TRUE(profiler.get("outer") >= profiler.get("inner"));
}
