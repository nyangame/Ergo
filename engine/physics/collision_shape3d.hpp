#pragma once
#include "../math/vec3.hpp"
#include <variant>

// 3D collision shapes for rigid body physics

struct SphereShape {
    float radius = 1.0f;
};

struct BoxShape {
    Vec3f half_extent{0.5f, 0.5f, 0.5f};
};

struct PlaneShape {
    Vec3f normal{0.0f, 1.0f, 0.0f};
    float offset = 0.0f;  // distance from origin along normal
};

using CollisionShape3D = std::variant<SphereShape, BoxShape, PlaneShape>;

// Contact information from collision detection
struct ContactPoint {
    Vec3f point;           // World-space contact point
    Vec3f normal;          // Contact normal (from A to B)
    float penetration;     // Penetration depth (positive = overlapping)
};
