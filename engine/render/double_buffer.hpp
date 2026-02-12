#pragma once
#include "command_buffer.hpp"
#include <atomic>
#include <array>

// Double-buffered command buffer system
// Front buffer: being consumed by the render thread
// Back buffer: being written to by game/worker threads
//
// At frame boundary, swap() exchanges front and back.
// This allows the render thread to process the previous frame's commands
// while the game threads build the next frame's commands concurrently.

class DoubleBufferedCommands {
    std::array<CommandBuffer, 2> buffers_;
    std::atomic<uint32_t> write_index_{0};

public:
    DoubleBufferedCommands() = default;

    // Get the buffer that game threads write into (back buffer)
    CommandBuffer& write_buffer() {
        return buffers_[write_index_.load(std::memory_order_acquire)];
    }

    // Get the buffer that the render thread reads from (front buffer)
    const CommandBuffer& read_buffer() const {
        return buffers_[1 - write_index_.load(std::memory_order_acquire)];
    }

    // Swap front and back buffers. Call at frame boundary.
    // The new back buffer is cleared for the next frame's commands.
    void swap() {
        uint32_t prev = write_index_.load(std::memory_order_acquire);
        uint32_t next = 1 - prev;
        write_index_.store(next, std::memory_order_release);
        buffers_[next].clear();
    }

    // Clear both buffers
    void clear() {
        buffers_[0].clear();
        buffers_[1].clear();
    }
};
