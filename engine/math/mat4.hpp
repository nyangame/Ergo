#pragma once
#include "vec3.hpp"
#include <cmath>
#include <cstring>

struct Mat4 {
    float m[16] = {};

    constexpr Mat4() { identity(); }

    constexpr void identity() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    constexpr float& operator()(int row, int col) { return m[col * 4 + row]; }
    constexpr float operator()(int row, int col) const { return m[col * 4 + row]; }

    constexpr Mat4 operator*(const Mat4& b) const {
        Mat4 r;
        for (int i = 0; i < 16; ++i) r.m[i] = 0.0f;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                for (int k = 0; k < 4; ++k) {
                    r.m[col * 4 + row] += m[k * 4 + row] * b.m[col * 4 + k];
                }
            }
        }
        return r;
    }

    constexpr Vec3f transform_point(Vec3f v) const {
        float w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15];
        return {
            (m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12]) / w,
            (m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13]) / w,
            (m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14]) / w
        };
    }

    constexpr Vec3f transform_direction(Vec3f v) const {
        return {
            m[0] * v.x + m[4] * v.y + m[8]  * v.z,
            m[1] * v.x + m[5] * v.y + m[9]  * v.z,
            m[2] * v.x + m[6] * v.y + m[10] * v.z
        };
    }

    static constexpr Mat4 translation(Vec3f t) {
        Mat4 r;
        r.m[12] = t.x; r.m[13] = t.y; r.m[14] = t.z;
        return r;
    }

    static constexpr Mat4 scale(Vec3f s) {
        Mat4 r;
        r.m[0] = s.x; r.m[5] = s.y; r.m[10] = s.z;
        return r;
    }

    static Mat4 rotation_y(float radians) {
        Mat4 r;
        float c = std::cos(radians);
        float s = std::sin(radians);
        r.m[0] = c;  r.m[8]  = s;
        r.m[2] = -s; r.m[10] = c;
        return r;
    }

    static Mat4 perspective(float fov_y, float aspect, float near_z, float far_z) {
        Mat4 r;
        for (int i = 0; i < 16; ++i) r.m[i] = 0.0f;
        float tan_half = std::tan(fov_y * 0.5f);
        r.m[0]  = 1.0f / (aspect * tan_half);
        r.m[5]  = 1.0f / tan_half;
        r.m[10] = -(far_z + near_z) / (far_z - near_z);
        r.m[11] = -1.0f;
        r.m[14] = -(2.0f * far_z * near_z) / (far_z - near_z);
        return r;
    }

    static Mat4 look_at(Vec3f eye, Vec3f target, Vec3f up) {
        Vec3f f = (target - eye).normalized();
        Vec3f s = f.cross(up).normalized();
        Vec3f u = s.cross(f);
        Mat4 r;
        r.m[0] = s.x;  r.m[4] = s.y;  r.m[8]  = s.z;
        r.m[1] = u.x;  r.m[5] = u.y;  r.m[9]  = u.z;
        r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
        r.m[12] = -s.dot(eye);
        r.m[13] = -u.dot(eye);
        r.m[14] = f.dot(eye);
        return r;
    }
};
