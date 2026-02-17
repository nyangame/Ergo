#pragma once
#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

// ============================================================
// JobSystem: generic worker thread pool for data-parallel work
//
// Designed for data-oriented processing where work is split
// into contiguous chunks that can be processed independently
// on separate cache lines.
//
// Usage:
//   g_job_system.parallel_for(0, count, 256, [&](uint32_t begin, uint32_t end) {
//       for (uint32_t i = begin; i < end; ++i) { ... }
//   });
// ============================================================

class JobSystem {
public:
    JobSystem() = default;
    ~JobSystem();

    // Initialize with explicit thread count (0 = auto-detect)
    void initialize(uint32_t thread_count = 0);
    void shutdown();

    // Parallel for: splits [begin, end) into chunks and dispatches to workers.
    // chunk_size controls granularity - align to cache line multiples for DOD.
    // The callback receives [chunk_begin, chunk_end) for each chunk.
    void parallel_for(uint32_t begin, uint32_t end, uint32_t chunk_size,
                      std::function<void(uint32_t, uint32_t)> fn);

    // Submit a single job and wait for all pending jobs to finish
    void submit(std::function<void()> fn);
    void wait();

    uint32_t worker_count() const { return static_cast<uint32_t>(workers_.size()); }
    bool is_active() const { return !shutdown_.load(std::memory_order_acquire); }

private:
    struct Job {
        std::function<void()> work;
    };

    std::vector<std::thread> workers_;
    std::vector<Job> queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<uint32_t> jobs_remaining_{0};
    std::mutex done_mutex_;
    std::condition_variable done_cv_;
    std::atomic<bool> shutdown_{false};

    void worker_func();
};

// Global instance (follows g_physics / g_time pattern)
inline JobSystem g_job_system;
