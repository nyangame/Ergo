#pragma once
#include "engine/core/concepts.hpp"
#include "engine/core/game_object.hpp"
#include "engine/physics/collider.hpp"
#include "game_types.hpp"

struct RenderContext;

struct Enemy {
    GameObject object;
    Collider collider;
    int hp_ = 3;
    float speed_ = 2.0f;
    bool alive_ = true;

    // Satisfies TaskLike concept
    void start();
    void update(float dt);
    void release();

    // Drawable
    void draw(RenderContext& ctx);

    // Collider callback
    bool hit_callback(const Collider& target);
};

static_assert(TaskLike<Enemy>);
