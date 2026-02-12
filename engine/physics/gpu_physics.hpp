#pragma once
#include "rigid_body.hpp"
#include "collision_shape3d.hpp"
#include "../math/vec3.hpp"
#include <vector>
#include <cstdint>
#include <functional>

// GPU-based physics component (Compute Shader backend)
// Runs rigid body simulation on GPU using compute shaders
// Suitable for: large body counts, particle-like physics, platforms with compute support
//
// Architecture:
//   1. Upload body state (position, velocity, forces) to GPU buffers
//   2. Dispatch compute shaders for integration + broadphase + narrowphase
//   3. Readback resolved positions/velocities
//
// Falls back to CPU if compute is unavailable

// GPU buffer handles (opaque IDs managed by the backend)
struct GpuBufferHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

// Compact body representation for GPU transfer (SOA-friendly)
struct alignas(16) GpuBodyData {
    float pos_x, pos_y, pos_z, inv_mass;
    float vel_x, vel_y, vel_z, restitution;
    float force_x, force_y, force_z, padding;
};

// Compact shape representation for GPU
struct alignas(16) GpuShapeData {
    uint32_t type;         // 0=sphere, 1=box, 2=plane
    float param0;          // sphere: radius, box: half_extent.x, plane: normal.x
    float param1;          // box: half_extent.y, plane: normal.y
    float param2;          // box: half_extent.z, plane: normal.z
};

class GpuPhysicsComponent {
    // GPU resource handles
    GpuBufferHandle body_buffer_;
    GpuBufferHandle shape_buffer_;
    GpuBufferHandle contact_buffer_;
    GpuBufferHandle dispatch_params_;

    // CPU-side shadow data
    std::vector<GpuBodyData> body_data_;
    std::vector<GpuShapeData> shape_data_;
    std::vector<uint64_t> body_ids_;

    uint64_t next_id_ = 1;
    Vec3f gravity_{0.0f, -9.81f, 0.0f};
    float fixed_dt_ = 1.0f / 60.0f;
    float accumulator_ = 0.0f;
    bool compute_available_ = false;
    bool initialized_ = false;

    // Collision callback (invoked after readback)
    struct GpuCollisionCallback {
        uint64_t body_id;
        std::function<void(uint64_t other_id, const ContactPoint& contact)> callback;
    };
    std::vector<GpuCollisionCallback> callbacks_;

    // CPU fallback
    void step_cpu(float dt);
    void integrate_cpu(float dt);
    void detect_collisions_cpu();

    // GPU execution
    void upload_to_gpu();
    void dispatch_compute();
    void readback_from_gpu();

public:
    GpuPhysicsComponent() = default;
    ~GpuPhysicsComponent();

    // Lifecycle
    void start();
    void update(float dt);
    void release();

    // Configuration
    void set_gravity(Vec3f g) { gravity_ = g; }
    void set_fixed_timestep(float dt) { fixed_dt_ = dt; }

    // Body management
    uint64_t add_body(Vec3f position, float mass, CollisionShape3D shape);
    void remove_body(uint64_t id);

    // Set collision callback
    void set_collision_callback(uint64_t body_id,
        std::function<void(uint64_t other_id, const ContactPoint& contact)> cb);

    // Apply force/impulse to a body
    void apply_force(uint64_t id, Vec3f force);
    void apply_impulse(uint64_t id, Vec3f impulse);

    // Query
    Vec3f get_position(uint64_t id) const;
    Vec3f get_velocity(uint64_t id) const;
    size_t body_count() const { return body_data_.size(); }
    bool is_compute_available() const { return compute_available_; }
};
