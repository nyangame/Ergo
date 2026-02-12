#pragma once
#include <variant>
#include <functional>
#include <cstdint>
#include "../math/vec2.hpp"
#include "../math/transform.hpp"

enum class ColliderTag : uint32_t {
    Invalid = 0,
    Player,
    Enemy,
    Bullet,
    Max
};

struct AABBData {
    Vec2f half_extent;
};

struct CircleData {
    float radius;
};

using ColliderShape = std::variant<AABBData, CircleData>;

struct ColliderHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

struct Collider {
    ColliderHandle handle;
    ColliderShape shape;
    ColliderTag tag = ColliderTag::Invalid;
    uint64_t owner_id = 0;
    const Transform2D* transform = nullptr;
    std::function<bool(const Collider&)> on_hit;
};
