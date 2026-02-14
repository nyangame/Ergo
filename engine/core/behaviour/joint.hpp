#pragma once
#include <string>
#include <string_view>
#include <variant>
#include <cmath>
#include "behaviour.hpp"
#include "../../math/vec2.hpp"
#include "../../math/transform.hpp"

// ============================================================
// Joint: connects two transforms with physical constraints
//   GUI properties: type, offset, stiffness, damping, limits
//   Types: Fixed, Spring, Hinge (variant-based, no inheritance)
// ============================================================

// --- Joint type data ---

struct FixedJointData {
    // Maintains exact relative offset
};

struct SpringJointData {
    float stiffness = 100.0f;     // spring constant (N/m)
    float damping = 5.0f;         // damping coefficient
    float rest_length = 0.0f;     // natural length (0 = auto from initial distance)
};

struct HingeJointData {
    float min_angle = -3.14159f;  // lower rotation limit (radians)
    float max_angle =  3.14159f;  // upper rotation limit (radians)
    float angular_damping = 1.0f;
};

using JointType = std::variant<FixedJointData, SpringJointData, HingeJointData>;

// ============================================================
// Joint behaviour
// ============================================================

struct Joint {
    // --- GUI-configurable properties ---
    JointType type = FixedJointData{};
    Vec2f offset;                // attachment offset from owner
    Vec2f target_offset;         // attachment offset from target
    bool break_on_force = false;
    float break_force = 1000.0f;

    // Connected transforms
    Transform2D* owner = nullptr;
    const Transform2D* target = nullptr;

    // --- Internal state ---
    Vec2f velocity_ = Vec2f::zero();
    float initial_distance_ = -1.0f;

    // --- BehaviourLike interface ---
    static constexpr std::string_view type_name() { return "Joint"; }

    void start() {
        velocity_ = Vec2f::zero();
        initial_distance_ = -1.0f;

        if (owner && target) {
            Vec2f owner_anchor = owner->position + offset;
            Vec2f target_anchor = target->position + target_offset;
            initial_distance_ = (target_anchor - owner_anchor).length();

            // Auto-set spring rest length if not specified
            std::visit([this](auto& j) {
                using J = std::decay_t<decltype(j)>;
                if constexpr (std::is_same_v<J, SpringJointData>) {
                    if (j.rest_length <= 0.0f)
                        j.rest_length = initial_distance_;
                }
            }, type);
        }
    }

    void update(float dt) {
        if (!owner || !target) return;

        std::visit([this, dt](auto& j) {
            using J = std::decay_t<decltype(j)>;

            if constexpr (std::is_same_v<J, FixedJointData>) {
                update_fixed(dt);
            }
            else if constexpr (std::is_same_v<J, SpringJointData>) {
                update_spring(j, dt);
            }
            else if constexpr (std::is_same_v<J, HingeJointData>) {
                update_hinge(j, dt);
            }
        }, type);
    }

    void release() {
        owner = nullptr;
        target = nullptr;
    }

    // --- Query API ---
    bool is_fixed() const {
        return std::holds_alternative<FixedJointData>(type);
    }
    bool is_spring() const {
        return std::holds_alternative<SpringJointData>(type);
    }
    bool is_hinge() const {
        return std::holds_alternative<HingeJointData>(type);
    }

    float current_distance() const {
        if (!owner || !target) return 0.0f;
        Vec2f owner_anchor = owner->position + offset;
        Vec2f target_anchor = target->position + target_offset;
        return (target_anchor - owner_anchor).length();
    }

private:
    void update_fixed(float /*dt*/) {
        Vec2f target_pos = target->position + target_offset - offset;
        owner->position = target_pos;
    }

    void update_spring(SpringJointData& spring, float dt) {
        Vec2f owner_anchor = owner->position + offset;
        Vec2f target_anchor = target->position + target_offset;
        Vec2f diff = target_anchor - owner_anchor;
        float dist = diff.length();

        if (dist < 0.0001f) return;

        Vec2f dir = diff * (1.0f / dist);
        float displacement = dist - spring.rest_length;

        // Hooke's law: F = -kx - cv
        float spring_force = spring.stiffness * displacement;
        float damp_force = spring.damping * dot(velocity_, dir);
        float total_force = spring_force - damp_force;

        velocity_ += dir * (total_force * dt);
        owner->position += velocity_ * dt;
    }

    void update_hinge(HingeJointData& hinge, float dt) {
        // Constrain owner position to orbit around target at initial_distance_
        Vec2f target_anchor = target->position + target_offset;
        Vec2f diff = owner->position + offset - target_anchor;
        float angle = std::atan2(diff.y, diff.x);

        // Clamp angle within limits
        if (angle < hinge.min_angle) angle = hinge.min_angle;
        if (angle > hinge.max_angle) angle = hinge.max_angle;

        float radius = (initial_distance_ > 0.0f) ? initial_distance_ : 50.0f;
        owner->position.x = target_anchor.x + std::cos(angle) * radius - offset.x;
        owner->position.y = target_anchor.y + std::sin(angle) * radius - offset.y;
        owner->rotation = angle;

        (void)dt;
        (void)hinge.angular_damping;
    }

    static float dot(Vec2f a, Vec2f b) {
        return a.x * b.x + a.y * b.y;
    }
};
