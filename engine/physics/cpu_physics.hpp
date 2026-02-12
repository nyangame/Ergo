#pragma once
#include "rigid_body_world.hpp"
#include "../core/concepts.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <condition_variable>

// CPU-based physics component
// Runs rigid body simulation on CPU threads
// Suitable for: precise simulation, small-medium body counts, all platforms

class CpuPhysicsComponent {
    RigidBodyWorld world_;

    // Multi-threaded broadphase partitioning
    struct WorkerContext {
        std::thread thread;
        std::atomic<bool> active{false};
    };
    std::vector<WorkerContext> workers_;
    std::mutex mutex_;
    std::atomic<bool> running_{false};

    uint32_t thread_count_ = 1;

public:
    CpuPhysicsComponent() = default;
    ~CpuPhysicsComponent();

    // Lifecycle (satisfies concept-based interface)
    void start();
    void update(float dt);
    void release();

    // Configuration
    void set_thread_count(uint32_t count);
    void set_gravity(Vec3f gravity) { world_.set_gravity(gravity); }

    // Body management (delegates to RigidBodyWorld)
    uint64_t add_body(PhysicsBody body) { return world_.add_body(std::move(body)); }
    void remove_body(uint64_t id) { world_.remove_body(id); }
    PhysicsBody* get_body(uint64_t id) { return world_.get_body(id); }

    RigidBodyWorld& world() { return world_; }
    const RigidBodyWorld& world() const { return world_; }
};
