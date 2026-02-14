#include "demo_framework.hpp"
#include "engine/core/time.hpp"
#include <cstdio>

DEMO(Time_FrameTick) {
    Time time;
    time.reset();

    std::printf("  Initial: dt=%.4f total=%.4f frame=%llu fps=%.1f\n",
                time.delta_time, time.total_time,
                (unsigned long long)time.frame_count, time.fps);

    // Simulate 60fps frames
    for (int i = 0; i < 5; ++i) {
        time.tick(1.0f / 60.0f);
        std::printf("    Frame %llu: dt=%.6f total=%.4f fps=%.1f\n",
                    (unsigned long long)time.frame_count,
                    time.delta_time, time.total_time, time.fps);
    }
}

DEMO(Time_TimeScale) {
    Time time;
    time.reset();

    time.time_scale = 0.5f;  // Half speed
    time.tick(1.0f / 60.0f);
    std::printf("  Half speed: raw_dt=%.6f scaled_dt=%.6f\n",
                time.unscaled_delta_time, time.delta_time);

    time.time_scale = 2.0f;  // Double speed
    time.tick(1.0f / 60.0f);
    std::printf("  Double speed: raw_dt=%.6f scaled_dt=%.6f\n",
                time.unscaled_delta_time, time.delta_time);

    time.time_scale = 0.0f;  // Paused
    time.tick(1.0f / 60.0f);
    std::printf("  Paused: raw_dt=%.6f scaled_dt=%.6f\n",
                time.unscaled_delta_time, time.delta_time);
}
