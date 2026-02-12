#include "cpu_physics.hpp"
#include <algorithm>

CpuPhysicsComponent::~CpuPhysicsComponent() {
    release();
}

void CpuPhysicsComponent::start() {
    running_.store(true);
    if (thread_count_ == 0) {
        thread_count_ = std::max(1u, std::thread::hardware_concurrency() / 2);
    }
}

void CpuPhysicsComponent::update(float dt) {
    if (!running_.load()) return;

    // For CPU physics, run the world step directly
    // The RigidBodyWorld handles its own fixed-timestep accumulation
    world_.step(dt);
}

void CpuPhysicsComponent::release() {
    running_.store(false);
    for (auto& w : workers_) {
        w.active.store(false);
        if (w.thread.joinable()) w.thread.join();
    }
    workers_.clear();
}

void CpuPhysicsComponent::set_thread_count(uint32_t count) {
    thread_count_ = (count == 0)
        ? std::max(1u, std::thread::hardware_concurrency() / 2)
        : count;
}
