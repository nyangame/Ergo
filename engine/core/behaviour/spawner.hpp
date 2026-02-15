#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <cstdint>
#include "behaviour.hpp"
#include "../../math/vec2.hpp"
#include "../../math/transform.hpp"

// ============================================================
// Spawner: periodically creates objects in a defined area
//   GUI properties: interval, max count, spawn area, factory
// ============================================================

struct SpawnParams {
    Vec2f position;
    float rotation = 0.0f;
    uint32_t index = 0;    // spawn sequence number
};

struct Spawner {
    // --- GUI-configurable properties ---
    float interval = 1.0f;         // seconds between spawns
    uint32_t max_count = 0;        // 0 = unlimited
    uint32_t burst_count = 1;      // objects per spawn event
    bool auto_start = true;
    bool loop = true;

    // Spawn area (relative to owner position)
    Vec2f area_min = {-50.0f, -50.0f};
    Vec2f area_max = { 50.0f,  50.0f};

    // Owner transform (set by holder)
    const Transform2D* owner_transform = nullptr;

    // Factory: called when spawning
    std::function<void(const SpawnParams&)> on_spawn;

    // Event callbacks
    std::function<void()> on_finished;

    // --- Internal state ---
    float timer_ = 0.0f;
    uint32_t total_spawned_ = 0;
    bool active_ = false;
    uint32_t seed_ = 12345;

    // --- BehaviourLike interface ---
    static constexpr std::string_view type_name() { return "Spawner"; }

    void start() {
        timer_ = 0.0f;
        total_spawned_ = 0;
        active_ = auto_start;
    }

    void update(float dt) {
        if (!active_) return;
        if (max_count > 0 && total_spawned_ >= max_count) {
            active_ = false;
            if (on_finished) on_finished();
            return;
        }

        timer_ += dt;
        if (timer_ < interval) return;
        timer_ -= interval;

        uint32_t count = burst_count;
        if (max_count > 0) {
            uint32_t remaining = max_count - total_spawned_;
            if (count > remaining) count = remaining;
        }

        for (uint32_t i = 0; i < count; ++i) {
            SpawnParams params;
            params.position = random_position_in_area();
            params.rotation = 0.0f;
            params.index = total_spawned_;

            if (on_spawn) on_spawn(params);
            ++total_spawned_;
        }
    }

    void release() {
        on_spawn = nullptr;
        on_finished = nullptr;
        owner_transform = nullptr;
    }

    // --- Control API ---
    void activate() { active_ = true; }
    void deactivate() { active_ = false; }
    void reset() { timer_ = 0.0f; total_spawned_ = 0; active_ = auto_start; }
    bool is_active() const { return active_; }
    uint32_t spawned_count() const { return total_spawned_; }

private:
    // Simple xorshift for deterministic random within area
    uint32_t next_random() {
        seed_ ^= seed_ << 13;
        seed_ ^= seed_ >> 17;
        seed_ ^= seed_ << 5;
        return seed_;
    }

    float random_float(float min_val, float max_val) {
        uint32_t r = next_random();
        float t = static_cast<float>(r & 0xFFFF) / 65535.0f;
        return min_val + t * (max_val - min_val);
    }

    Vec2f random_position_in_area() {
        Vec2f base = owner_transform ? owner_transform->position : Vec2f::zero();
        return {
            base.x + random_float(area_min.x, area_max.x),
            base.y + random_float(area_min.y, area_max.y)
        };
    }
};
