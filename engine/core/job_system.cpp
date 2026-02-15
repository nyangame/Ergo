#include "job_system.hpp"
#include <algorithm>

JobSystem::~JobSystem() {
    shutdown();
}

void JobSystem::initialize(uint32_t thread_count) {
    if (!workers_.empty()) return;  // already initialized

    if (thread_count == 0) {
        thread_count = std::max(1u, std::thread::hardware_concurrency() - 1);
    }

    shutdown_.store(false, std::memory_order_release);
    workers_.reserve(thread_count);
    for (uint32_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back(&JobSystem::worker_func, this);
    }
}

void JobSystem::shutdown() {
    shutdown_.store(true, std::memory_order_release);
    queue_cv_.notify_all();
    for (auto& w : workers_) {
        if (w.joinable()) w.join();
    }
    workers_.clear();
}

void JobSystem::worker_func() {
    while (!shutdown_.load(std::memory_order_acquire)) {
        Job job;
        {
            std::unique_lock lock(queue_mutex_);
            queue_cv_.wait(lock, [this] {
                return !queue_.empty() || shutdown_.load(std::memory_order_acquire);
            });

            if (shutdown_.load(std::memory_order_acquire) && queue_.empty()) return;
            if (queue_.empty()) continue;

            job = std::move(queue_.back());
            queue_.pop_back();
        }

        if (job.work) {
            job.work();
        }

        if (jobs_remaining_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            std::lock_guard lock(done_mutex_);
            done_cv_.notify_all();
        }
    }
}

void JobSystem::parallel_for(uint32_t begin, uint32_t end, uint32_t chunk_size,
                              std::function<void(uint32_t, uint32_t)> fn) {
    if (begin >= end) return;
    if (chunk_size == 0) chunk_size = 1;

    uint32_t total = end - begin;

    // For small workloads or no workers, run inline
    if (total <= chunk_size || workers_.empty()) {
        fn(begin, end);
        return;
    }

    // Split into chunks
    uint32_t chunk_count = (total + chunk_size - 1) / chunk_size;

    jobs_remaining_.store(chunk_count, std::memory_order_release);

    {
        std::lock_guard lock(queue_mutex_);
        for (uint32_t c = 0; c < chunk_count; ++c) {
            uint32_t cb = begin + c * chunk_size;
            uint32_t ce = std::min(cb + chunk_size, end);
            queue_.push_back({[fn, cb, ce]() { fn(cb, ce); }});
        }
    }
    queue_cv_.notify_all();

    // Wait for completion
    wait();
}

void JobSystem::submit(std::function<void()> fn) {
    jobs_remaining_.fetch_add(1, std::memory_order_acq_rel);
    {
        std::lock_guard lock(queue_mutex_);
        queue_.push_back({std::move(fn)});
    }
    queue_cv_.notify_one();
}

void JobSystem::wait() {
    std::unique_lock lock(done_mutex_);
    done_cv_.wait(lock, [this] {
        return jobs_remaining_.load(std::memory_order_acquire) == 0;
    });
}
