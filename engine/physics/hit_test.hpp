#pragma once
#include "collider.hpp"

// AABB vs AABB
bool hit_test(const AABBData& a, const Transform2D& ta,
              const AABBData& b, const Transform2D& tb);

// Circle vs Circle
bool hit_test(const CircleData& a, const Transform2D& ta,
              const CircleData& b, const Transform2D& tb);

// Circle vs AABB
bool hit_test(const CircleData& circle, const Transform2D& tc,
              const AABBData& aabb, const Transform2D& ta);

// AABB vs Circle (argument order reversed)
bool hit_test(const AABBData& aabb, const Transform2D& ta,
              const CircleData& circle, const Transform2D& tc);

// Generic check using variant visitor
bool check_hit(const Collider& a, const Collider& b);
