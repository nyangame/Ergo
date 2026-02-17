#pragma once
#include <string_view>
#include <functional>
#include <optional>
#include <vector>
#include "behaviour.hpp"
#include "../../math/vec2.hpp"
#include "../../math/transform.hpp"
#include "../../render/particle_system.hpp"

// ============================================================
// ParticleEmitterComponent: attaches a particle emitter to a
//   GameObject via the behaviour composition system.
//   Synchronizes emitter position with owner transform each frame.
//   GUI properties: emitter config, auto-play, position offset
// ============================================================

struct ParticleEmitterComponent {
    // --- GUI-configurable properties ---
    EmitterConfig config;
    Vec2f offset;                          // local offset from owner transform
    bool auto_play = true;                 // start emitting on start()
    bool follow_owner = true;              // continuously track owner position

    // Owner transform (set by holder / game code)
    const Transform2D* owner_transform = nullptr;

    // Event callbacks
    std::function<void()> on_finished;     // called when non-looping emitter dies

    // --- BehaviourLike + ThreadAware interface ---
    static constexpr std::string_view type_name() { return "ParticleEmitterComponent"; }
    static constexpr ThreadingPolicy threading_policy() { return ThreadingPolicy::MainThread; }

    void start() {
        emitter_ = std::nullopt;
        finished_notified_ = false;

        sync_position();
        emitter_.emplace(config);

        if (auto_play) {
            emitter_->start();
        } else {
            emitter_->stop();
        }
    }

    void update(float dt) {
        if (!emitter_) return;

        if (follow_owner) {
            sync_position();
            emitter_->set_position(config.position);
        }

        emitter_->update(dt);

        // Notify when a non-looping emitter finishes
        if (!config.loop && !emitter_->is_alive() && !finished_notified_) {
            finished_notified_ = true;
            if (on_finished) on_finished();
        }
    }

    void draw(RenderContext& ctx) {
        if (emitter_) {
            emitter_->draw(ctx);
        }
    }

    void release() {
        emitter_ = std::nullopt;
        owner_transform = nullptr;
        on_finished = nullptr;
    }

    // --- Control API ---
    void play() {
        if (emitter_) emitter_->start();
    }

    void stop() {
        if (emitter_) emitter_->stop();
    }

    void burst(uint32_t count) {
        if (emitter_) emitter_->burst(count);
    }

    bool is_alive() const {
        return emitter_ && emitter_->is_alive();
    }

    bool is_playing() const {
        return emitter_.has_value();
    }

    void restart() {
        start();
    }

    // Access to underlying emitter (advanced usage)
    ParticleEmitter* emitter() {
        return emitter_ ? &(*emitter_) : nullptr;
    }

    const ParticleEmitter* emitter() const {
        return emitter_ ? &(*emitter_) : nullptr;
    }

private:
    std::optional<ParticleEmitter> emitter_;
    bool finished_notified_ = false;

    void sync_position() {
        Vec2f base = owner_transform ? owner_transform->position : Vec2f::zero();
        config.position = base + offset;
    }
};
