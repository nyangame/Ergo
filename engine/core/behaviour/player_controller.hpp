#pragma once
#include <string>
#include <string_view>
#include <functional>
#include "behaviour.hpp"
#include "../../math/vec2.hpp"
#include "../../math/transform.hpp"

// ============================================================
// PlayerController: input-driven movement behaviour
//   GUI properties: speed, input bindings, callbacks
// ============================================================

struct PlayerController {
    // --- GUI-configurable properties ---
    float move_speed = 200.0f;
    float jump_force = 400.0f;
    bool use_gravity = false;
    float gravity = 980.0f;

    // Input action names (bound via InputMap)
    std::string action_move_left  = "move_left";
    std::string action_move_right = "move_right";
    std::string action_move_up    = "move_up";
    std::string action_move_down  = "move_down";
    std::string action_jump       = "jump";
    std::string action_fire       = "fire";

    // Target transform (set by holder)
    Transform2D* target = nullptr;

    // Input query functions (injected by engine)
    std::function<bool(std::string_view)> is_action_down;
    std::function<bool(std::string_view)> is_action_pressed;

    // Event callbacks
    std::function<void()> on_jump;
    std::function<void()> on_fire;
    std::function<void(Vec2f)> on_move;

    // --- Internal state ---
    Vec2f velocity;

    // --- BehaviourLike + ThreadAware interface ---
    static constexpr std::string_view type_name() { return "PlayerController"; }
    static constexpr ThreadingPolicy threading_policy() { return ThreadingPolicy::MainThread; }

    void start() {
        velocity = Vec2f::zero();
    }

    void update(float dt) {
        if (!target) return;

        Vec2f input_dir = Vec2f::zero();

        if (is_action_down) {
            if (is_action_down(action_move_left))  input_dir.x -= 1.0f;
            if (is_action_down(action_move_right)) input_dir.x += 1.0f;
            if (is_action_down(action_move_up))    input_dir.y -= 1.0f;
            if (is_action_down(action_move_down))  input_dir.y += 1.0f;
        }

        // Normalize diagonal movement
        if (input_dir.length_sq() > 0.0f)
            input_dir = input_dir.normalized();

        velocity.x = input_dir.x * move_speed;

        if (use_gravity) {
            velocity.y += gravity * dt;
        } else {
            velocity.y = input_dir.y * move_speed;
        }

        // Jump (only when gravity is active)
        if (is_action_pressed && is_action_pressed(action_jump) && use_gravity) {
            velocity.y = -jump_force;
            if (on_jump) on_jump();
        }

        // Fire
        if (is_action_pressed && is_action_pressed(action_fire)) {
            if (on_fire) on_fire();
        }

        target->position += velocity * dt;

        if (on_move && input_dir.length_sq() > 0.0f) {
            on_move(input_dir);
        }
    }

    void release() {
        target = nullptr;
        on_jump = nullptr;
        on_fire = nullptr;
        on_move = nullptr;
    }
};
