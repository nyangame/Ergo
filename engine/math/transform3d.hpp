#pragma once
#include "vec3.hpp"
#include "quat.hpp"
#include "mat4.hpp"

struct Transform3D {
    Vec3f position;
    Quat rotation;
    Vec3f scale_{1.0f, 1.0f, 1.0f};

    Mat4 to_mat4() const {
        Mat4 t = Mat4::translation(position);
        Mat4 r = rotation.to_mat4();
        Mat4 s = Mat4::scale(scale_);
        return t * r * s;
    }
};
