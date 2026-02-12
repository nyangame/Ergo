#include "test_framework.hpp"
#include "engine/math/transform.hpp"
#include "engine/math/transform3d.hpp"

TEST_CASE(Transform2D_Default) {
    Transform2D t;
    ASSERT_NEAR(t.position.x, 0.0f, 0.001f);
    ASSERT_NEAR(t.position.y, 0.0f, 0.001f);
    ASSERT_NEAR(t.rotation, 0.0f, 0.001f);
    ASSERT_NEAR(t.size.w, 0.0f, 0.001f);
    ASSERT_NEAR(t.size.h, 0.0f, 0.001f);
}

TEST_CASE(Transform2D_SetValues) {
    Transform2D t;
    t.position = {100.0f, 200.0f};
    t.rotation = 3.14f;
    t.size = {64.0f, 48.0f};
    ASSERT_NEAR(t.position.x, 100.0f, 0.001f);
    ASSERT_NEAR(t.position.y, 200.0f, 0.001f);
    ASSERT_NEAR(t.rotation, 3.14f, 0.001f);
    ASSERT_NEAR(t.size.w, 64.0f, 0.001f);
}

TEST_CASE(Transform3D_Default) {
    Transform3D t;
    ASSERT_NEAR(t.position.x, 0.0f, 0.001f);
    ASSERT_NEAR(t.scale_.x, 1.0f, 0.001f);
    ASSERT_NEAR(t.scale_.y, 1.0f, 0.001f);
    ASSERT_NEAR(t.scale_.z, 1.0f, 0.001f);
}

TEST_CASE(Transform3D_ToMat4_Translation) {
    Transform3D t;
    t.position = {5.0f, 10.0f, 15.0f};
    t.rotation = Quat::identity();
    t.scale_ = {1.0f, 1.0f, 1.0f};

    Mat4 m = t.to_mat4();
    ASSERT_NEAR(m.m[12], 5.0f, 0.001f);
    ASSERT_NEAR(m.m[13], 10.0f, 0.001f);
    ASSERT_NEAR(m.m[14], 15.0f, 0.001f);
}

TEST_CASE(Transform3D_ToMat4_Scale) {
    Transform3D t;
    t.position = {0.0f, 0.0f, 0.0f};
    t.rotation = Quat::identity();
    t.scale_ = {2.0f, 3.0f, 4.0f};

    Mat4 m = t.to_mat4();
    ASSERT_NEAR(m.m[0], 2.0f, 0.001f);
    ASSERT_NEAR(m.m[5], 3.0f, 0.001f);
    ASSERT_NEAR(m.m[10], 4.0f, 0.001f);
}
