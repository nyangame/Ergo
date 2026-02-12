#pragma once
#include <cmath>

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2f() = default;
    constexpr Vec2f(float x, float y) : x(x), y(y) {}

    constexpr Vec2f operator+(Vec2f o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2f operator-(Vec2f o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2f operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2f& operator+=(Vec2f o) { x += o.x; y += o.y; return *this; }
    constexpr Vec2f& operator-=(Vec2f o) { x -= o.x; y -= o.y; return *this; }
    constexpr Vec2f& operator*=(float s) { x *= s; y *= s; return *this; }

    constexpr float length_sq() const { return x * x + y * y; }
    float length() const { return std::sqrt(length_sq()); }
    Vec2f normalized() const {
        float l = length();
        return (l > 0.0f) ? Vec2f{x / l, y / l} : Vec2f{};
    }

    static constexpr Vec2f zero() { return {0.0f, 0.0f}; }
};
