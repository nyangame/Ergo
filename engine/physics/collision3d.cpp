#include "collision3d.hpp"
#include <cmath>
#include <algorithm>

std::optional<ContactPoint> collide_sphere_sphere(
    const SphereShape& a, const Transform3D& ta,
    const SphereShape& b, const Transform3D& tb)
{
    Vec3f diff = tb.position - ta.position;
    float dist_sq = diff.length_sq();
    float r_sum = a.radius + b.radius;

    if (dist_sq >= r_sum * r_sum) return std::nullopt;

    float dist = std::sqrt(dist_sq);
    Vec3f normal = (dist > 0.0001f) ? diff * (1.0f / dist) : Vec3f{0.0f, 1.0f, 0.0f};

    return ContactPoint{
        ta.position + normal * a.radius,
        normal,
        r_sum - dist
    };
}

std::optional<ContactPoint> collide_sphere_plane(
    const SphereShape& sphere, const Transform3D& ts,
    const PlaneShape& plane)
{
    float dist = plane.normal.dot(ts.position) - plane.offset;

    if (dist >= sphere.radius) return std::nullopt;

    return ContactPoint{
        ts.position - plane.normal * dist,
        plane.normal,
        sphere.radius - dist
    };
}

std::optional<ContactPoint> collide_sphere_box(
    const SphereShape& sphere, const Transform3D& ts,
    const BoxShape& box, const Transform3D& tb)
{
    // Transform sphere center into box local space
    Vec3f local = ts.position - tb.position;
    local = tb.rotation.conjugate().rotate(local);

    // Clamp to box extents to find closest point
    Vec3f closest{
        std::clamp(local.x, -box.half_extent.x, box.half_extent.x),
        std::clamp(local.y, -box.half_extent.y, box.half_extent.y),
        std::clamp(local.z, -box.half_extent.z, box.half_extent.z)
    };

    Vec3f diff = local - closest;
    float dist_sq = diff.length_sq();

    if (dist_sq >= sphere.radius * sphere.radius) return std::nullopt;

    float dist = std::sqrt(dist_sq);
    Vec3f local_normal = (dist > 0.0001f) ? diff * (1.0f / dist) : Vec3f{0.0f, 1.0f, 0.0f};
    Vec3f world_normal = tb.rotation.rotate(local_normal);
    Vec3f world_closest = tb.position + tb.rotation.rotate(closest);

    return ContactPoint{
        world_closest,
        world_normal,
        sphere.radius - dist
    };
}

std::optional<ContactPoint> collide_box_plane(
    const BoxShape& box, const Transform3D& tb,
    const PlaneShape& plane)
{
    // Check all 8 corners of the box against the plane
    Vec3f corners[8] = {
        {-box.half_extent.x, -box.half_extent.y, -box.half_extent.z},
        { box.half_extent.x, -box.half_extent.y, -box.half_extent.z},
        {-box.half_extent.x,  box.half_extent.y, -box.half_extent.z},
        { box.half_extent.x,  box.half_extent.y, -box.half_extent.z},
        {-box.half_extent.x, -box.half_extent.y,  box.half_extent.z},
        { box.half_extent.x, -box.half_extent.y,  box.half_extent.z},
        {-box.half_extent.x,  box.half_extent.y,  box.half_extent.z},
        { box.half_extent.x,  box.half_extent.y,  box.half_extent.z},
    };

    float deepest = 0.0f;
    Vec3f deepest_point;
    bool found = false;

    for (auto& c : corners) {
        Vec3f world = tb.position + tb.rotation.rotate(c);
        float dist = plane.normal.dot(world) - plane.offset;
        if (dist < deepest || !found) {
            deepest = dist;
            deepest_point = world;
            found = true;
        }
    }

    if (deepest >= 0.0f) return std::nullopt;

    return ContactPoint{
        deepest_point,
        plane.normal,
        -deepest
    };
}

std::optional<ContactPoint> collide_box_box(
    const BoxShape& a, const Transform3D& ta,
    const BoxShape& b, const Transform3D& tb)
{
    // Simplified AABB vs AABB (ignoring rotation for initial implementation)
    Vec3f diff = tb.position - ta.position;
    Vec3f overlap{
        a.half_extent.x + b.half_extent.x - std::abs(diff.x),
        a.half_extent.y + b.half_extent.y - std::abs(diff.y),
        a.half_extent.z + b.half_extent.z - std::abs(diff.z)
    };

    if (overlap.x <= 0.0f || overlap.y <= 0.0f || overlap.z <= 0.0f)
        return std::nullopt;

    // Find minimum penetration axis
    Vec3f normal;
    float penetration;

    if (overlap.x < overlap.y && overlap.x < overlap.z) {
        penetration = overlap.x;
        normal = {(diff.x > 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f};
    } else if (overlap.y < overlap.z) {
        penetration = overlap.y;
        normal = {0.0f, (diff.y > 0.0f) ? 1.0f : -1.0f, 0.0f};
    } else {
        penetration = overlap.z;
        normal = {0.0f, 0.0f, (diff.z > 0.0f) ? 1.0f : -1.0f};
    }

    Vec3f point = ta.position + normal * (a.half_extent.x * normal.x +
                                           a.half_extent.y * normal.y +
                                           a.half_extent.z * normal.z);

    return ContactPoint{point, normal, penetration};
}

std::optional<ContactPoint> check_collision3d(
    const CollisionShape3D& shape_a, const Transform3D& ta,
    const CollisionShape3D& shape_b, const Transform3D& tb)
{
    return std::visit([&](const auto& a, const auto& b) -> std::optional<ContactPoint> {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        if constexpr (std::is_same_v<A, SphereShape> && std::is_same_v<B, SphereShape>) {
            return collide_sphere_sphere(a, ta, b, tb);
        } else if constexpr (std::is_same_v<A, SphereShape> && std::is_same_v<B, BoxShape>) {
            return collide_sphere_box(a, ta, b, tb);
        } else if constexpr (std::is_same_v<A, BoxShape> && std::is_same_v<B, SphereShape>) {
            auto r = collide_sphere_box(b, tb, a, ta);
            if (r) r->normal = r->normal * -1.0f;
            return r;
        } else if constexpr (std::is_same_v<A, SphereShape> && std::is_same_v<B, PlaneShape>) {
            return collide_sphere_plane(a, ta, b);
        } else if constexpr (std::is_same_v<A, PlaneShape> && std::is_same_v<B, SphereShape>) {
            auto r = collide_sphere_plane(b, tb, a);
            if (r) r->normal = r->normal * -1.0f;
            return r;
        } else if constexpr (std::is_same_v<A, BoxShape> && std::is_same_v<B, BoxShape>) {
            return collide_box_box(a, ta, b, tb);
        } else if constexpr (std::is_same_v<A, BoxShape> && std::is_same_v<B, PlaneShape>) {
            return collide_box_plane(a, ta, b);
        } else if constexpr (std::is_same_v<A, PlaneShape> && std::is_same_v<B, BoxShape>) {
            auto r = collide_box_plane(b, tb, a);
            if (r) r->normal = r->normal * -1.0f;
            return r;
        } else {
            // Plane vs Plane: no collision
            return std::nullopt;
        }
    }, shape_a, shape_b);
}
