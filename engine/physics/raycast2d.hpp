#pragma once
#include "../math/vec2.hpp"
#include "collider.hpp"
#include <optional>
#include <vector>

struct RayHit2D {
    Vec2f point;
    Vec2f normal;
    float distance = 0.0f;
    const Collider* collider = nullptr;
};

// Cast a ray and return the first hit
std::optional<RayHit2D> raycast2d(
    Vec2f origin, Vec2f direction, float max_distance,
    const std::vector<Collider*>& colliders,
    ColliderTag mask = ColliderTag::Invalid);

// Cast a ray and return all hits sorted by distance
std::vector<RayHit2D> raycast2d_all(
    Vec2f origin, Vec2f direction, float max_distance,
    const std::vector<Collider*>& colliders,
    ColliderTag mask = ColliderTag::Invalid);
