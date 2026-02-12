#include "hit_test.hpp"
#include <cmath>

// Ported from CppSampleGame's IsHitCircle
static bool is_hit_circle(Vec2f a, Vec2f b, float r) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy < r * r;
}

bool hit_test(const AABBData& a, const Transform2D& ta,
              const AABBData& b, const Transform2D& tb) {
    float xa1 = ta.position.x - a.half_extent.x;
    float xa2 = ta.position.x + a.half_extent.x;
    float ya1 = ta.position.y - a.half_extent.y;
    float ya2 = ta.position.y + a.half_extent.y;
    float xb1 = tb.position.x - b.half_extent.x;
    float xb2 = tb.position.x + b.half_extent.x;
    float yb1 = tb.position.y - b.half_extent.y;
    float yb2 = tb.position.y + b.half_extent.y;
    return xa1 <= xb2 && xa2 >= xb1 && ya1 <= yb2 && ya2 >= yb1;
}

bool hit_test(const CircleData& a, const Transform2D& ta,
              const CircleData& b, const Transform2D& tb) {
    return is_hit_circle(ta.position, tb.position, a.radius + b.radius);
}

// Ported from CppSampleGame's IsHitCircleToAABB
bool hit_test(const CircleData& circle, const Transform2D& tc,
              const AABBData& aabb, const Transform2D& ta) {
    float cx = tc.position.x;
    float cy = tc.position.y;
    float r = circle.radius;

    float x1 = ta.position.x - aabb.half_extent.x;
    float x2 = ta.position.x + aabb.half_extent.x;
    float y1 = ta.position.y - aabb.half_extent.y;
    float y2 = ta.position.y + aabb.half_extent.y;

    // Extended rectangle vs point check
    if (x1 - r < cx && cx < x2 + r && y1 < cy && cy < y2) return true;
    if (x1 < cx && cx < x2 && y1 - r < cy && cy < y2 + r) return true;

    // Corner circle checks
    Vec2f pos = tc.position;
    if (is_hit_circle({x1, y1}, pos, r)) return true;
    if (is_hit_circle({x2, y1}, pos, r)) return true;
    if (is_hit_circle({x1, y2}, pos, r)) return true;
    if (is_hit_circle({x2, y2}, pos, r)) return true;

    return false;
}

bool hit_test(const AABBData& aabb, const Transform2D& ta,
              const CircleData& circle, const Transform2D& tc) {
    return hit_test(circle, tc, aabb, ta);
}

bool check_hit(const Collider& a, const Collider& b) {
    return std::visit([&](const auto& sa, const auto& sb) {
        return hit_test(sa, *a.transform, sb, *b.transform);
    }, a.shape, b.shape);
}
