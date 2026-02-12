#pragma once
#include <array>
#include <vector>
#include <utility>
#include "collider.hpp"

class PhysicsSystem {
    std::array<std::vector<Collider*>,
               static_cast<size_t>(ColliderTag::Max)> colliders_;
    std::vector<Collider*> calc_stack_;
    std::vector<std::pair<Collider*, ColliderTag>> remove_list_;
    uint64_t next_id_ = 1;

    // Check one collider against all colliders of a target tag
    void hit_to_all(Collider* c, ColliderTag target_tag);

public:
    PhysicsSystem();

    ColliderHandle register_collider(Collider& c);
    void remove_collider(Collider& c);
    void mark_moved(Collider& c);  // CppSampleGame CalcStack equivalent
    void run();                     // Execute collision detection + remove processing
};

// Global instance (Singleton<T> replacement)
inline PhysicsSystem g_physics;
