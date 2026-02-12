#include "physics_system.hpp"
#include "hit_test.hpp"
#include <algorithm>

PhysicsSystem::PhysicsSystem() {
    for (auto& v : colliders_) {
        v.reserve(64);
    }
    calc_stack_.reserve(64);
}

ColliderHandle PhysicsSystem::register_collider(Collider& c) {
    uint64_t id = next_id_++;
    c.handle = {id};
    auto tag_idx = static_cast<size_t>(c.tag);
    if (tag_idx < colliders_.size()) {
        colliders_[tag_idx].push_back(&c);
    }
    return c.handle;
}

void PhysicsSystem::remove_collider(Collider& c) {
    remove_list_.emplace_back(&c, c.tag);
}

void PhysicsSystem::mark_moved(Collider& c) {
    calc_stack_.push_back(&c);
}

void PhysicsSystem::hit_to_all(Collider* c, ColliderTag target_tag) {
    auto tag_idx = static_cast<size_t>(target_tag);
    if (tag_idx >= colliders_.size()) return;

    for (auto* target : colliders_[tag_idx]) {
        if (c == target) continue;
        if (c->handle.id == 0 || target->handle.id == 0) continue;

        if (check_hit(*c, *target)) {
            bool consumed = false;
            if (c->on_hit) {
                consumed = c->on_hit(*target);
            }
            if (!consumed && target->on_hit) {
                target->on_hit(*c);
            }
        }
    }
}

void PhysicsSystem::run() {
    // Process collision detection for moved objects
    for (auto* c : calc_stack_) {
        if (c->handle.id == 0) continue;

        // Check against all relevant tags
        // Player vs Enemy, Player vs Bullet, Enemy vs Bullet, etc.
        for (size_t i = 0; i < static_cast<size_t>(ColliderTag::Max); ++i) {
            auto target_tag = static_cast<ColliderTag>(i);
            if (target_tag == c->tag) continue;
            if (target_tag == ColliderTag::Invalid) continue;
            hit_to_all(c, target_tag);
        }
    }
    calc_stack_.clear();

    // Process deferred removals
    for (auto& [collider, tag] : remove_list_) {
        auto tag_idx = static_cast<size_t>(tag);
        if (tag_idx < colliders_.size()) {
            auto& vec = colliders_[tag_idx];
            vec.erase(
                std::remove(vec.begin(), vec.end(), collider),
                vec.end()
            );
        }
        collider->handle = {0};
    }
    remove_list_.clear();
}
