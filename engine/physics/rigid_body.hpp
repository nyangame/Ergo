#pragma once
#include "../math/vec3.hpp"
#include "../math/quat.hpp"
#include "../math/transform3d.hpp"
#include <cstdint>

// Rigid body type: static objects never move, dynamic objects are simulated
enum class RigidBodyType : uint8_t {
    Static,
    Dynamic
};

// Rigid body component for 3D physics simulation (collision + gravity/falling)
struct RigidBody {
    RigidBodyType type = RigidBodyType::Dynamic;

    // Physical properties
    float mass = 1.0f;
    float inv_mass = 1.0f;       // Precomputed 1/mass (0 for static)
    float restitution = 0.3f;    // Bounciness [0..1]
    float friction = 0.5f;       // Surface friction [0..1]

    // Linear dynamics
    Vec3f velocity;
    Vec3f acceleration;
    Vec3f force_accumulator;

    // Angular dynamics
    Vec3f angular_velocity;
    Vec3f torque_accumulator;

    // Damping to stabilize simulation
    float linear_damping = 0.01f;
    float angular_damping = 0.05f;

    // Gravity scale (0 = no gravity, 1 = normal)
    float gravity_scale = 1.0f;

    // Sleeping (optimization for resting bodies)
    bool is_sleeping = false;
    float sleep_timer = 0.0f;

    // Link to transform (non-owning)
    Transform3D* transform = nullptr;
    uint64_t owner_id = 0;

    void set_mass(float m) {
        mass = m;
        inv_mass = (type == RigidBodyType::Static || m <= 0.0f) ? 0.0f : 1.0f / m;
    }

    void set_static() {
        type = RigidBodyType::Static;
        inv_mass = 0.0f;
        velocity = Vec3f::zero();
        angular_velocity = Vec3f::zero();
    }

    void apply_force(Vec3f f) {
        if (type == RigidBodyType::Static) return;
        force_accumulator += f;
        wake();
    }

    void apply_impulse(Vec3f impulse) {
        if (type == RigidBodyType::Static) return;
        velocity += impulse * inv_mass;
        wake();
    }

    void apply_torque(Vec3f t) {
        if (type == RigidBodyType::Static) return;
        torque_accumulator += t;
        wake();
    }

    void wake() {
        is_sleeping = false;
        sleep_timer = 0.0f;
    }

    void clear_forces() {
        force_accumulator = Vec3f::zero();
        torque_accumulator = Vec3f::zero();
    }
};
