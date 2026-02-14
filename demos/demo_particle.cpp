#include "demo_framework.hpp"
#include "engine/render/particle_system.hpp"
#include <cstdio>

DEMO(ParticleEmitter_Burst) {
    EmitterConfig config;
    config.position = {100.0f, 100.0f};
    config.emit_rate = 0.0f;  // No continuous emission
    config.particle_life_min = 0.5f;
    config.particle_life_max = 1.0f;
    config.velocity_min = {-50.0f, -100.0f};
    config.velocity_max = {50.0f, -50.0f};
    config.color_start = {255, 200, 0, 255};
    config.color_end = {255, 0, 0, 0};
    config.size_start = 8.0f;
    config.size_end = 2.0f;
    config.max_particles = 100;

    ParticleEmitter emitter(config);
    emitter.burst(20);
    std::printf("  Burst 20 particles at (100, 100)\n");
    std::printf("  Alive after burst: %s\n", emitter.is_alive() ? "yes" : "no");

    for (int i = 0; i < 5; ++i) {
        emitter.update(0.2f);
        std::printf("    t=%.1f alive=%s\n", 0.2f * (i + 1),
                    emitter.is_alive() ? "yes" : "no");
    }
}

DEMO(ParticleManager_Multiple) {
    ParticleManager mgr;

    EmitterConfig fire;
    fire.position = {200.0f, 300.0f};
    fire.emit_rate = 50.0f;
    fire.particle_life_min = 0.3f;
    fire.particle_life_max = 0.8f;
    fire.max_particles = 200;
    mgr.add(fire);

    EmitterConfig smoke;
    smoke.position = {200.0f, 280.0f};
    smoke.emit_rate = 20.0f;
    smoke.particle_life_min = 1.0f;
    smoke.particle_life_max = 2.0f;
    smoke.max_particles = 100;
    mgr.add(smoke);

    std::printf("  Emitters: %zu\n", mgr.emitter_count());

    for (int i = 0; i < 3; ++i) {
        mgr.update(0.016f);
    }
    std::printf("  Updated 3 frames\n");

    mgr.clear();
    std::printf("  After clear: %zu emitters\n", mgr.emitter_count());
}
