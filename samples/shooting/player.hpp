#pragma once
#include "engine/core/concepts.hpp"
#include "engine/core/game_object.hpp"
#include "engine/physics/collider.hpp"
#include "game_types.hpp"

// Forward declaration
struct RenderContext;

struct Player {
    // Data
    GameObject object;
    Collider collider;
    int hp_ = 100;
    int interval_ = 0;
    float ground_y_ = 0.0f;
    float jump_pow_ = 25.0f;
    float jump_y_ = 0.0f;
    float grav_ = 0.0f;

    // Satisfies TaskLike concept
    void start();
    void update(float dt);
    void release();

    // Drawable
    void draw(RenderContext& ctx);

    // Collider callback
    bool hit_callback(const Collider& target);
};

// Concept verification
static_assert(TaskLike<Player>);
