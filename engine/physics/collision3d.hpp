#pragma once
#include "collision_shape3d.hpp"
#include "../math/transform3d.hpp"
#include <optional>

// 3D collision detection functions
// Returns contact information if shapes overlap

std::optional<ContactPoint> collide_sphere_sphere(
    const SphereShape& a, const Transform3D& ta,
    const SphereShape& b, const Transform3D& tb);

std::optional<ContactPoint> collide_sphere_box(
    const SphereShape& sphere, const Transform3D& ts,
    const BoxShape& box, const Transform3D& tb);

std::optional<ContactPoint> collide_sphere_plane(
    const SphereShape& sphere, const Transform3D& ts,
    const PlaneShape& plane);

std::optional<ContactPoint> collide_box_box(
    const BoxShape& a, const Transform3D& ta,
    const BoxShape& b, const Transform3D& tb);

std::optional<ContactPoint> collide_box_plane(
    const BoxShape& box, const Transform3D& tb,
    const PlaneShape& plane);

// Generic collision check using variant visitor
std::optional<ContactPoint> check_collision3d(
    const CollisionShape3D& shape_a, const Transform3D& ta,
    const CollisionShape3D& shape_b, const Transform3D& tb);
