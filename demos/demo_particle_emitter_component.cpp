#include "demo_framework.hpp"
#include "engine/core/behaviour/particle_emitter_component.hpp"
#include "engine/core/behaviour/behaviour_registry.hpp"
#include "engine/math/transform.hpp"
#include <cstdio>

// ============================================================
// ParticleEmitterComponent demos
// ============================================================

DEMO(ParticleEmitterComponent_BasicUsage) {
    // Attach a fire-like emitter to a game object's transform
    Transform2D obj_transform;
    obj_transform.position = {200.0f, 300.0f};
    obj_transform.rotation = 0.0f;
    obj_transform.size = {32.0f, 32.0f};

    ParticleEmitterComponent comp;
    comp.config.emit_rate = 30.0f;
    comp.config.particle_life_min = 0.3f;
    comp.config.particle_life_max = 0.8f;
    comp.config.velocity_min = {-20.0f, -80.0f};
    comp.config.velocity_max = {20.0f, -40.0f};
    comp.config.color_start = {255, 200, 50, 255};
    comp.config.color_end = {255, 50, 0, 0};
    comp.config.size_start = 6.0f;
    comp.config.size_end = 1.0f;
    comp.config.max_particles = 200;
    comp.owner_transform = &obj_transform;
    comp.auto_play = true;

    comp.start();
    std::printf("  Started emitter at (%.0f, %.0f)\n",
                obj_transform.position.x, obj_transform.position.y);
    std::printf("  Alive: %s\n", comp.is_alive() ? "yes" : "no");

    // Simulate a few frames
    for (int i = 0; i < 5; ++i) {
        comp.update(0.016f);
    }
    std::printf("  After 5 frames: alive=%s\n", comp.is_alive() ? "yes" : "no");

    comp.release();
    std::printf("  Released\n");
}

DEMO(ParticleEmitterComponent_FollowOwner) {
    Transform2D obj_transform;
    obj_transform.position = {100.0f, 100.0f};

    ParticleEmitterComponent comp;
    comp.config.emit_rate = 20.0f;
    comp.config.max_particles = 100;
    comp.offset = {0.0f, -16.0f};  // emit above the object
    comp.follow_owner = true;
    comp.owner_transform = &obj_transform;

    comp.start();
    std::printf("  Initial position: (%.0f, %.0f)\n",
                obj_transform.position.x, obj_transform.position.y);

    // Move the object and update — emitter should follow
    obj_transform.position = {300.0f, 200.0f};
    comp.update(0.016f);

    const auto& cfg = comp.emitter()->config();
    std::printf("  After move: owner=(%.0f, %.0f) emitter=(%.0f, %.0f)\n",
                obj_transform.position.x, obj_transform.position.y,
                cfg.position.x, cfg.position.y);
    std::printf("  Expected emitter at (300, 184) with offset (0, -16)\n");

    comp.release();
}

DEMO(ParticleEmitterComponent_BurstAndStop) {
    ParticleEmitterComponent comp;
    comp.config.emit_rate = 0.0f;  // no continuous emission
    comp.config.particle_life_min = 0.2f;
    comp.config.particle_life_max = 0.5f;
    comp.config.max_particles = 50;
    comp.config.loop = false;
    comp.auto_play = false;

    bool finished = false;
    comp.on_finished = [&finished]() { finished = true; };

    comp.start();
    std::printf("  Alive before burst: %s\n", comp.is_alive() ? "yes" : "no");

    comp.burst(10);
    std::printf("  Burst 10 particles, alive: %s\n", comp.is_alive() ? "yes" : "no");

    // Simulate until all particles die
    for (int i = 0; i < 60 && comp.is_alive(); ++i) {
        comp.update(0.016f);
    }
    std::printf("  After simulation: alive=%s, on_finished called=%s\n",
                comp.is_alive() ? "yes" : "no",
                finished ? "yes" : "no");

    comp.release();
}

DEMO(ParticleEmitterComponent_BehaviourHolder) {
    // Demonstrate attaching via BehaviourHolder (same as other behaviours)
    BehaviourHolder holder;

    auto& emitter = holder.add<ParticleEmitterComponent>();
    emitter.config.emit_rate = 10.0f;
    emitter.config.max_particles = 50;

    holder.start();
    std::printf("  Holder has ParticleEmitterComponent: %s\n",
                holder.has<ParticleEmitterComponent>() ? "yes" : "no");

    auto* found = holder.get<ParticleEmitterComponent>();
    std::printf("  Retrieved via get<>: %s\n", found != nullptr ? "yes" : "no");
    std::printf("  Is alive: %s\n", found->is_alive() ? "yes" : "no");

    holder.update(0.016f);
    holder.update(0.016f);
    std::printf("  Updated 2 frames OK\n");

    holder.release();
    std::printf("  Released holder\n");
}

DEMO(ParticleEmitterComponent_Registry) {
    // Register and create via BehaviourRegistry
    BehaviourRegistry registry;
    registry.register_type<ParticleEmitterComponent>("Effects");

    auto names = registry.names_in_category("Effects");
    std::printf("  Effects category has %zu behaviours\n", names.size());

    auto behaviour = registry.create("ParticleEmitterComponent");
    std::printf("  Created from registry: %s\n", behaviour ? "yes" : "no");
    std::printf("  Type name: %.*s\n",
                static_cast<int>(behaviour->type_name().size()),
                behaviour->type_name().data());

    // Verify threading policy
    const auto* entry = registry.find("ParticleEmitterComponent");
    std::printf("  Thread aware: %s\n", entry->thread_aware ? "yes" : "no");
    std::printf("  Policy: MainThread=%s\n",
                entry->policy == ThreadingPolicy::MainThread ? "yes" : "no");
}
