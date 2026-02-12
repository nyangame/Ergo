#pragma once
#include "../math/vec2.hpp"
#include "../math/color.hpp"
#include "../resource/texture_handle.hpp"
#include <vector>
#include <cstdint>
#include <functional>

struct Particle {
    Vec2f position;
    Vec2f velocity;
    Color color;
    float life = 0.0f;
    float max_life = 1.0f;
    float size = 1.0f;
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
};

struct EmitterConfig {
    Vec2f position;
    float emit_rate = 10.0f;
    float particle_life_min = 0.5f;
    float particle_life_max = 1.5f;
    Vec2f velocity_min{-50.0f, -50.0f};
    Vec2f velocity_max{50.0f, 50.0f};
    Color color_start{255, 255, 255, 255};
    Color color_end{255, 255, 255, 0};
    float size_start = 8.0f;
    float size_end = 2.0f;
    Vec2f gravity;
    TextureHandle texture;
    uint32_t max_particles = 1000;
    bool loop = true;
};

struct RenderContext;

class ParticleEmitter {
public:
    explicit ParticleEmitter(EmitterConfig config);

    void update(float dt);
    void draw(RenderContext& ctx);

    void start() { active_ = true; }
    void stop() { active_ = false; }
    void burst(uint32_t count);
    bool is_alive() const;

    void set_position(Vec2f pos) { config_.position = pos; }
    const EmitterConfig& config() const { return config_; }

private:
    EmitterConfig config_;
    std::vector<Particle> particles_;
    float emit_accumulator_ = 0.0f;
    bool active_ = true;

    void emit_particle();
    static float random_range(float min, float max);
    static Color lerp_color(Color a, Color b, float t);
};

class ParticleManager {
public:
    ParticleEmitter& add(EmitterConfig config);
    void update(float dt);
    void draw(RenderContext& ctx);
    void clear();
    size_t emitter_count() const { return emitters_.size(); }

private:
    std::vector<ParticleEmitter> emitters_;
};

inline ParticleManager g_particles;
