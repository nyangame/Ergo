#pragma once
#include "render_command.hpp"
#include <vector>
#include <mutex>
#include <cstdint>

// Thread-safe command buffer for accumulating render commands from multiple threads
// Each worker thread can have its own CommandBuffer, then merge into the main one

class CommandBuffer {
    std::vector<RenderCommand> commands_;
    uint32_t sort_key_ = 0;  // For command ordering

public:
    CommandBuffer() { commands_.reserve(1024); }

    void clear() {
        commands_.clear();
        sort_key_ = 0;
    }

    void push(RenderCommand cmd) {
        commands_.push_back(std::move(cmd));
    }

    template<typename T>
    void push(T&& cmd) {
        commands_.push_back(RenderCommand{std::forward<T>(cmd)});
    }

    // Merge another buffer's commands (for multi-thread collection)
    void merge(const CommandBuffer& other) {
        commands_.insert(commands_.end(),
                         other.commands_.begin(), other.commands_.end());
    }

    const std::vector<RenderCommand>& commands() const { return commands_; }
    size_t size() const { return commands_.size(); }
    bool empty() const { return commands_.empty(); }
};

// Thread-safe wrapper for merging command buffers from multiple worker threads
class SharedCommandCollector {
    std::mutex mutex_;
    CommandBuffer merged_;

public:
    void submit(const CommandBuffer& buffer) {
        std::lock_guard lock(mutex_);
        merged_.merge(buffer);
    }

    CommandBuffer take() {
        std::lock_guard lock(mutex_);
        CommandBuffer result;
        std::swap(result, merged_);
        return result;
    }

    void clear() {
        std::lock_guard lock(mutex_);
        merged_.clear();
    }
};
