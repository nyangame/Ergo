#pragma once
#include "vec2.hpp"
#include "size2.hpp"

struct Transform2D {
    Vec2f position;
    float rotation = 0.0f;  // radians
    Size2f size;
};
