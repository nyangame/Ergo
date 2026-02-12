#include "particle_system.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"
#include <cstdlib>
#include <algorithm>
#include <cmath>

// ParticleEmitter
ParticleEmitter::ParticleEmitter(EmitterConfig config)
    : config_(std::move(config)) {
    particles_.reserve(config_.max_particles);
}

float ParticleEmitter::random_range(float min, float max) {
    float r = static_cast<float>(std::rand()) / RAND_MAX;
    return min + r * (max - min);
}

Color ParticleEmitter::lerp_color(Color a, Color b, float t) {
    return {
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<uint8_t>(a.a + (b.a - a.a) * t)
    };
}

void ParticleEmitter::emit_particle() {
    if (particles_.size() >= config_.max_particles) return;

    Particle p;
    p.position = config_.position;
    p.velocity = {
        random_range(config_.velocity_min.x, config_.velocity_max.x),
        random_range(config_.velocity_min.y, config_.velocity_max.y)
    };
    p.color = config_.color_start;
    p.life = 0.0f;
    p.max_life = random_range(config_.particle_life_min, config_.particle_life_max);
    p.size = config_.size_start;
    particles_.push_back(p);
}

void ParticleEmitter::update(float dt) {
    // Emit new particles
    if (active_) {
        emit_accumulator_ += config_.emit_rate * dt;
        while (emit_accumulator_ >= 1.0f) {
            emit_particle();
            emit_accumulator_ -= 1.0f;
        }
    }

    // Update existing particles
    for (auto& p : particles_) {
        p.life += dt;
        float t = p.life / p.max_life;

        p.velocity += config_.gravity * dt;
        p.position += p.velocity * dt;
        p.rotation += p.rotation_speed * dt;

        p.color = lerp_color(config_.color_start, config_.color_end, t);
        p.size = config_.size_start + (config_.size_end - config_.size_start) * t;
    }

    // Remove dead particles
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const Particle& p) { return p.life >= p.max_life; }),
        particles_.end());

    // Stop non-looping emitter when all particles are dead
    if (!config_.loop && particles_.empty() && !active_) {
        // Fully dead
    }
}

void ParticleEmitter::draw(RenderContext& ctx) {
    for (const auto& p : particles_) {
        float hs = p.size * 0.5f;
        if (config_.texture.valid()) {
            ctx.draw_sprite(
                {p.position.x - hs, p.position.y - hs},
                {p.size, p.size},
                config_.texture,
                {0, 0, 1, 1});
        } else {
            ctx.draw_circle(p.position, hs, p.color, true);
        }
    }
}

void ParticleEmitter::burst(uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        emit_particle();
    }
}

bool ParticleEmitter::is_alive() const {
    return active_ || !particles_.empty();
}

// ParticleManager
ParticleEmitter& ParticleManager::add(EmitterConfig config) {
    emitters_.emplace_back(std::move(config));
    return emitters_.back();
}

void ParticleManager::update(float dt) {
    for (auto& e : emitters_) {
        e.update(dt);
    }
    // Remove dead non-looping emitters
    emitters_.erase(
        std::remove_if(emitters_.begin(), emitters_.end(),
            [](const ParticleEmitter& e) { return !e.is_alive(); }),
        emitters_.end());
}

void ParticleManager::draw(RenderContext& ctx) {
    for (auto& e : emitters_) {
        e.draw(ctx);
    }
}

void ParticleManager::clear() {
    emitters_.clear();
}
