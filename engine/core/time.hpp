#pragma once
#include <cstdint>
#include <chrono>
#include <thread>

struct Time {
    float delta_time = 0.0f;
    float unscaled_delta_time = 0.0f;
    float total_time = 0.0f;
    float time_scale = 1.0f;
    float fixed_delta_time = 1.0f / 60.0f;
    uint64_t frame_count = 0;
    float fps = 0.0f;

    void tick(float raw_dt) {
        unscaled_delta_time = raw_dt;
        delta_time = raw_dt * time_scale;
        total_time += delta_time;
        ++frame_count;
        float alpha = 0.1f;
        float instant_fps = (raw_dt > 0.0f) ? 1.0f / raw_dt : 0.0f;
        fps = fps * (1.0f - alpha) + instant_fps * alpha;
    }

    void reset() {
        delta_time = 0.0f;
        unscaled_delta_time = 0.0f;
        total_time = 0.0f;
        time_scale = 1.0f;
        frame_count = 0;
        fps = 0.0f;
    }
};

struct FrameRateLimiter {
    float target_fps = 60.0f;

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point frame_start_;

    void begin_frame() {
        frame_start_ = Clock::now();
    }

    void wait() {
        if (target_fps <= 0.0f) return;
        auto target_duration = std::chrono::duration<float>(1.0f / target_fps);
        auto elapsed = Clock::now() - frame_start_;
        if (elapsed < target_duration) {
            std::this_thread::sleep_until(frame_start_ +
                std::chrono::duration_cast<Clock::duration>(target_duration));
        }
    }
};

inline Time g_time;
