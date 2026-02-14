#include "demo_framework.hpp"
#include "engine/debug/profiler.hpp"
#include <cstdio>
#include <thread>
#include <chrono>

DEMO(Profiler_Scoped) {
    Profiler profiler;

    profiler.begin("Outer");
    {
        profiler.begin("Inner_A");
        // Simulate work
        volatile int sum = 0;
        for (int i = 0; i < 100000; ++i) sum += i;
        profiler.end();

        profiler.begin("Inner_B");
        for (int i = 0; i < 50000; ++i) sum += i;
        profiler.end();
    }
    profiler.end();

    std::printf("  Profile results:\n");
    for (auto& [name, ms] : profiler.results()) {
        std::printf("    %-12s : %.4f ms\n", name.c_str(), ms);
    }
}

DEMO(Profiler_GlobalInstance) {
    g_profiler.clear();

    {
        ERGO_PROFILE_SCOPE("demo_scope");
        volatile int x = 0;
        for (int i = 0; i < 10000; ++i) x += i;
    }

    float ms = g_profiler.get("demo_scope");
    std::printf("  g_profiler 'demo_scope': %.4f ms\n", ms);
}
