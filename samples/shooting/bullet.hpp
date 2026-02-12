#pragma once
#include "engine/core/concepts.hpp"
#include "engine/core/game_object.hpp"
#include "engine/physics/collider.hpp"
#include "game_types.hpp"

struct RenderContext;

struct Bullet {
    GameObject object;
    Collider collider;
    Vec2f velocity_;
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

static_assert(TaskLike<Bullet>);
