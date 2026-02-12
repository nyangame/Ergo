#pragma once
#include <cmath>

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3f() = default;
    constexpr Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

    constexpr Vec3f operator+(Vec3f o) const { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vec3f operator-(Vec3f o) const { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vec3f operator*(float s) const { return {x * s, y * s, z * s}; }
    constexpr Vec3f operator/(float s) const { return {x / s, y / s, z / s}; }
    constexpr Vec3f& operator+=(Vec3f o) { x += o.x; y += o.y; z += o.z; return *this; }
    constexpr Vec3f& operator-=(Vec3f o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    constexpr Vec3f& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    constexpr float length_sq() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(length_sq()); }
    Vec3f normalized() const {
        float l = length();
        return (l > 0.0f) ? Vec3f{x / l, y / l, z / l} : Vec3f{};
    }

    constexpr float dot(Vec3f o) const { return x * o.x + y * o.y + z * o.z; }
    constexpr Vec3f cross(Vec3f o) const {
        return {y * o.z - z * o.y,
                z * o.x - x * o.z,
                x * o.y - y * o.x};
    }

    static constexpr Vec3f zero() { return {0.0f, 0.0f, 0.0f}; }
    static constexpr Vec3f up() { return {0.0f, 1.0f, 0.0f}; }
    static constexpr Vec3f forward() { return {0.0f, 0.0f, -1.0f}; }
    static constexpr Vec3f right() { return {1.0f, 0.0f, 0.0f}; }
};
