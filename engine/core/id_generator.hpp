#pragma once
#include <cstdint>
#include <atomic>

struct IdGenerator {
    static uint64_t next() {
        static std::atomic<uint64_t> counter{1};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }
};
