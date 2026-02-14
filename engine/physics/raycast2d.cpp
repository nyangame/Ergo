#include "raycast2d.hpp"
#include <algorithm>
#include <cmath>

namespace {

// Ray vs AABB intersection
std::optional<float> ray_vs_aabb(Vec2f origin, Vec2f dir, Vec2f center, Vec2f half) {
    Vec2f min_p = {center.x - half.x, center.y - half.y};
    Vec2f max_p = {center.x + half.x, center.y + half.y};

    float tmin = -1e30f, tmax = 1e30f;

    if (std::abs(dir.x) > 1e-8f) {
        float t1 = (min_p.x - origin.x) / dir.x;
        float t2 = (max_p.x - origin.x) / dir.x;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
    } else {
        if (origin.x < min_p.x || origin.x > max_p.x) return std::nullopt;
    }

    if (std::abs(dir.y) > 1e-8f) {
        float t1 = (min_p.y - origin.y) / dir.y;
        float t2 = (max_p.y - origin.y) / dir.y;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
    } else {
        if (origin.y < min_p.y || origin.y > max_p.y) return std::nullopt;
    }

    if (tmin > tmax || tmax < 0.0f) return std::nullopt;
    return (tmin >= 0.0f) ? tmin : tmax;
}

// Ray vs Circle intersection
std::optional<float> ray_vs_circle(Vec2f origin, Vec2f dir, Vec2f center, float radius) {
    Vec2f oc = {origin.x - center.x, origin.y - center.y};
    float a = dir.x * dir.x + dir.y * dir.y;
    float b = 2.0f * (oc.x * dir.x + oc.y * dir.y);
    float c = oc.x * oc.x + oc.y * oc.y - radius * radius;
    float disc = b * b - 4.0f * a * c;
    if (disc < 0.0f) return std::nullopt;
    float sqrt_disc = std::sqrt(disc);
    float t1 = (-b - sqrt_disc) / (2.0f * a);
    float t2 = (-b + sqrt_disc) / (2.0f * a);
    if (t1 >= 0.0f) return t1;
    if (t2 >= 0.0f) return t2;
    return std::nullopt;
}

Vec2f compute_normal_aabb(Vec2f hit, Vec2f center, Vec2f half) {
    Vec2f d = {hit.x - center.x, hit.y - center.y};
    float ax = std::abs(d.x / half.x);
    float ay = std::abs(d.y / half.y);
    if (ax > ay) return {d.x > 0 ? 1.0f : -1.0f, 0.0f};
    return {0.0f, d.y > 0 ? 1.0f : -1.0f};
}

} // anonymous namespace

std::optional<RayHit2D> raycast2d(
    Vec2f origin, Vec2f direction, float max_distance,
    const std::vector<Collider*>& colliders,
    ColliderTag mask)
{
    auto results = raycast2d_all(origin, direction, max_distance, colliders, mask);
    if (results.empty()) return std::nullopt;
    return results.front();
}

std::vector<RayHit2D> raycast2d_all(
    Vec2f origin, Vec2f direction, float max_distance,
    const std::vector<Collider*>& colliders,
    ColliderTag mask)
{
    Vec2f dir = direction.normalized();
    std::vector<RayHit2D> results;

    for (const Collider* c : colliders) {
        if (!c || !c->transform) continue;
        if (mask != ColliderTag::Invalid && c->tag != mask) continue;

        Vec2f center = c->transform->position;
        std::optional<float> t;
        Vec2f normal;

        if (auto* aabb = std::get_if<AABBData>(&c->shape)) {
            t = ray_vs_aabb(origin, dir, center, aabb->half_extent);
            if (t && *t <= max_distance) {
                Vec2f hit = {origin.x + dir.x * *t, origin.y + dir.y * *t};
                normal = compute_normal_aabb(hit, center, aabb->half_extent);
            }
        } else if (auto* circle = std::get_if<CircleData>(&c->shape)) {
            t = ray_vs_circle(origin, dir, center, circle->radius);
            if (t && *t <= max_distance) {
                Vec2f hit = {origin.x + dir.x * *t, origin.y + dir.y * *t};
                Vec2f diff = {hit.x - center.x, hit.y - center.y};
                normal = diff.normalized();
            }
        }

        if (t && *t >= 0.0f && *t <= max_distance) {
            RayHit2D hit;
            hit.point = {origin.x + dir.x * *t, origin.y + dir.y * *t};
            hit.normal = normal;
            hit.distance = *t;
            hit.collider = c;
            results.push_back(hit);
        }
    }

    std::sort(results.begin(), results.end(),
              [](const RayHit2D& a, const RayHit2D& b) { return a.distance < b.distance; });
    return results;
}
