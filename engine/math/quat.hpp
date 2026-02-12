#pragma once
#include "vec3.hpp"
#include "mat4.hpp"
#include <cmath>

struct Quat {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quat() = default;
    constexpr Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    static Quat from_axis_angle(Vec3f axis, float radians) {
        float half = radians * 0.5f;
        float s = std::sin(half);
        Vec3f n = axis.normalized();
        return {n.x * s, n.y * s, n.z * s, std::cos(half)};
    }

    static constexpr Quat identity() { return {0.0f, 0.0f, 0.0f, 1.0f}; }

    constexpr Quat operator*(const Quat& q) const {
        return {
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        };
    }

    constexpr float length_sq() const { return x * x + y * y + z * z + w * w; }
    float length() const { return std::sqrt(length_sq()); }

    Quat normalized() const {
        float l = length();
        if (l > 0.0f) return {x / l, y / l, z / l, w / l};
        return identity();
    }

    constexpr Quat conjugate() const { return {-x, -y, -z, w}; }

    constexpr Vec3f rotate(Vec3f v) const {
        Quat p{v.x, v.y, v.z, 0.0f};
        Quat result = (*this) * p * conjugate();
        return {result.x, result.y, result.z};
    }

    Mat4 to_mat4() const {
        Mat4 r;
        float xx = x * x, yy = y * y, zz = z * z;
        float xy = x * y, xz = x * z, yz = y * z;
        float wx = w * x, wy = w * y, wz = w * z;
        r.m[0]  = 1.0f - 2.0f * (yy + zz);
        r.m[1]  = 2.0f * (xy + wz);
        r.m[2]  = 2.0f * (xz - wy);
        r.m[4]  = 2.0f * (xy - wz);
        r.m[5]  = 1.0f - 2.0f * (xx + zz);
        r.m[6]  = 2.0f * (yz + wx);
        r.m[8]  = 2.0f * (xz + wy);
        r.m[9]  = 2.0f * (yz - wx);
        r.m[10] = 1.0f - 2.0f * (xx + yy);
        return r;
    }

    static Quat slerp(Quat a, Quat b, float t) {
        float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        if (dot < 0.0f) {
            b = {-b.x, -b.y, -b.z, -b.w};
            dot = -dot;
        }
        if (dot > 0.9995f) {
            return Quat{
                a.x + t * (b.x - a.x),
                a.y + t * (b.y - a.y),
                a.z + t * (b.z - a.z),
                a.w + t * (b.w - a.w)
            }.normalized();
        }
        float theta = std::acos(dot);
        float sin_theta = std::sin(theta);
        float wa = std::sin((1.0f - t) * theta) / sin_theta;
        float wb = std::sin(t * theta) / sin_theta;
        return Quat{
            wa * a.x + wb * b.x,
            wa * a.y + wb * b.y,
            wa * a.z + wb * b.z,
            wa * a.w + wb * b.w
        };
    }
};
