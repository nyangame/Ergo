#include "demo_framework.hpp"
#include "engine/math/vec2.hpp"
#include "engine/math/vec3.hpp"
#include "engine/math/mat4.hpp"
#include "engine/math/quat.hpp"
#include "engine/math/color.hpp"
#include "engine/math/size2.hpp"
#include "engine/math/transform.hpp"
#include "engine/math/transform3d.hpp"
#include <cstdio>

DEMO(Vec2f_Operations) {
    Vec2f a{3.0f, 4.0f};
    Vec2f b{1.0f, 2.0f};

    Vec2f sum = a + b;
    std::printf("  a = (%.1f, %.1f)\n", a.x, a.y);
    std::printf("  b = (%.1f, %.1f)\n", b.x, b.y);
    std::printf("  a + b = (%.1f, %.1f)\n", sum.x, sum.y);
    std::printf("  a - b = (%.1f, %.1f)\n", (a - b).x, (a - b).y);
    std::printf("  a * 2 = (%.1f, %.1f)\n", (a * 2.0f).x, (a * 2.0f).y);
    std::printf("  |a| = %.4f\n", a.length());
    std::printf("  normalize(a) = (%.4f, %.4f)\n", a.normalized().x, a.normalized().y);
}

DEMO(Vec3f_Operations) {
    Vec3f a{1.0f, 0.0f, 0.0f};
    Vec3f b{0.0f, 1.0f, 0.0f};

    std::printf("  a = (%.1f, %.1f, %.1f)\n", a.x, a.y, a.z);
    std::printf("  b = (%.1f, %.1f, %.1f)\n", b.x, b.y, b.z);
    std::printf("  dot(a, b) = %.4f\n", a.dot(b));

    Vec3f cross = a.cross(b);
    std::printf("  cross(a, b) = (%.1f, %.1f, %.1f)\n", cross.x, cross.y, cross.z);
    std::printf("  up = (%.1f, %.1f, %.1f)\n", Vec3f::up().x, Vec3f::up().y, Vec3f::up().z);
}

DEMO(Mat4_Transforms) {
    Mat4 identity;
    std::printf("  Identity diagonal: [%.1f, %.1f, %.1f, %.1f]\n",
                identity.m[0], identity.m[5], identity.m[10], identity.m[15]);

    Mat4 t = Mat4::translation({10.0f, 20.0f, 30.0f});
    Vec3f point = t.transform_point({0.0f, 0.0f, 0.0f});
    std::printf("  Translate origin by (10,20,30): (%.1f, %.1f, %.1f)\n",
                point.x, point.y, point.z);

    Mat4 s = Mat4::scale({2.0f, 3.0f, 4.0f});
    Vec3f scaled = s.transform_point({1.0f, 1.0f, 1.0f});
    std::printf("  Scale (1,1,1) by (2,3,4): (%.1f, %.1f, %.1f)\n",
                scaled.x, scaled.y, scaled.z);

    Mat4 persp = Mat4::perspective(1.047f, 16.0f / 9.0f, 0.1f, 100.0f);
    std::printf("  Perspective m[0]=%.4f, m[5]=%.4f\n", persp.m[0], persp.m[5]);
}

DEMO(Quaternion_Rotation) {
    Quat q = Quat::from_axis_angle(Vec3f::up(), 3.14159f / 2.0f);
    std::printf("  90-degree rotation around Y: (%.4f, %.4f, %.4f, %.4f)\n",
                q.x, q.y, q.z, q.w);

    Vec3f rotated = q.rotate({1.0f, 0.0f, 0.0f});
    std::printf("  Rotate (1,0,0) by 90 deg Y: (%.4f, %.4f, %.4f)\n",
                rotated.x, rotated.y, rotated.z);

    Quat a = Quat::identity();
    Quat b = Quat::from_axis_angle(Vec3f::up(), 3.14159f);
    Quat mid = Quat::slerp(a, b, 0.5f);
    std::printf("  Slerp(identity, 180-deg, 0.5) = (%.4f, %.4f, %.4f, %.4f)\n",
                mid.x, mid.y, mid.z, mid.w);
}

DEMO(Color_And_Size) {
    Color red{255, 0, 0, 255};
    Color semi{128, 128, 128, 128};
    std::printf("  Red: (%d, %d, %d, %d)\n", red.r, red.g, red.b, red.a);
    std::printf("  Semi: (%d, %d, %d, %d)\n", semi.r, semi.g, semi.b, semi.a);

    Size2f s{100.0f, 60.0f};
    std::printf("  Size: %.1f x %.1f, half=(%.1f, %.1f)\n",
                s.w, s.h, s.half_w(), s.half_h());
}

DEMO(Transform2D_And_3D) {
    Transform2D t2d;
    t2d.position = {100.0f, 200.0f};
    t2d.rotation = 0.5f;
    t2d.size = {32.0f, 32.0f};
    std::printf("  Transform2D: pos=(%.1f, %.1f) rot=%.2f size=(%.1f, %.1f)\n",
                t2d.position.x, t2d.position.y, t2d.rotation, t2d.size.w, t2d.size.h);

    Transform3D t3d;
    t3d.position = {1.0f, 2.0f, 3.0f};
    t3d.rotation = Quat::from_axis_angle(Vec3f::up(), 0.785f);
    t3d.scale_ = {2.0f, 2.0f, 2.0f};
    Mat4 m = t3d.to_mat4();
    std::printf("  Transform3D -> Mat4 m[12..14]: (%.4f, %.4f, %.4f)\n",
                m.m[12], m.m[13], m.m[14]);
}
