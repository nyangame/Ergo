#pragma once
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"
#include <cmath>

struct Camera3D {
    Vec3f position;
    Vec3f target;
    Vec3f up{0.0f, 1.0f, 0.0f};
    float fov = 60.0f;       // degrees
    float near_z = 0.1f;
    float far_z = 1000.0f;
    float aspect = 16.0f / 9.0f;

    Mat4 view_matrix() const {
        return Mat4::look_at(position, target, up);
    }

    Mat4 projection_matrix() const {
        float fov_rad = fov * 3.14159265f / 180.0f;
        return Mat4::perspective(fov_rad, aspect, near_z, far_z);
    }

    Mat4 view_projection() const {
        return projection_matrix() * view_matrix();
    }

    Vec3f forward() const {
        return (target - position).normalized();
    }

    Vec3f right_dir() const {
        return forward().cross(up).normalized();
    }
};
