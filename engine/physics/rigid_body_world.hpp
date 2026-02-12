#pragma once
#include "rigid_body.hpp"
#include "collision_shape3d.hpp"
#include <vector>
#include <cstdint>
#include <functional>

// A body entry in the physics world, pairing a rigid body with its collision shape
struct PhysicsBody {
    uint64_t id = 0;
    RigidBody body;
    CollisionShape3D shape;
    Transform3D transform;

    // Collision callback: called when this body collides with another
    std::function<void(PhysicsBody& other, const ContactPoint& contact)> on_collision;
};

// Rigid body physics world
// Manages integration, collision detection, and collision response
class RigidBodyWorld {
    std::vector<PhysicsBody> bodies_;
    uint64_t next_id_ = 1;

    // World settings
    Vec3f gravity_{0.0f, -9.81f, 0.0f};
    float fixed_dt_ = 1.0f / 60.0f;
    float accumulator_ = 0.0f;
    int max_substeps_ = 4;

    // Sleep thresholds
    static constexpr float sleep_velocity_threshold_ = 0.05f;
    static constexpr float sleep_time_threshold_ = 0.5f;

    void integrate(float dt);
    void detect_and_resolve();
    void resolve_contact(PhysicsBody& a, PhysicsBody& b, const ContactPoint& contact);
    void update_sleep(float dt);

public:
    RigidBodyWorld() = default;

    void set_gravity(Vec3f g) { gravity_ = g; }
    Vec3f gravity() const { return gravity_; }

    void set_fixed_timestep(float dt) { fixed_dt_ = dt; }
    void set_max_substeps(int n) { max_substeps_ = n; }

    // Add a body and return its ID
    uint64_t add_body(PhysicsBody body);

    // Remove a body by ID
    void remove_body(uint64_t id);

    // Get a body by ID (nullptr if not found)
    PhysicsBody* get_body(uint64_t id);
    const PhysicsBody* get_body(uint64_t id) const;

    // Step the simulation with fixed-timestep accumulation
    void step(float dt);

    // Get all bodies (for rendering, etc.)
    const std::vector<PhysicsBody>& bodies() const { return bodies_; }
    std::vector<PhysicsBody>& bodies() { return bodies_; }

    size_t body_count() const { return bodies_.size(); }
};

// Global instance
inline RigidBodyWorld g_rigid_body_world;
