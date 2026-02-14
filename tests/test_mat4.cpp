#include "test_framework.hpp"
#include "engine/math/mat4.hpp"

TEST_CASE(mat4_identity) {
    Mat4 m;
    ASSERT_NEAR(m(0, 0), 1.0f, 1e-6f);
    ASSERT_NEAR(m(1, 1), 1.0f, 1e-6f);
    ASSERT_NEAR(m(2, 2), 1.0f, 1e-6f);
    ASSERT_NEAR(m(3, 3), 1.0f, 1e-6f);
    ASSERT_NEAR(m(0, 1), 0.0f, 1e-6f);
}

TEST_CASE(mat4_translation) {
    Mat4 t = Mat4::translation({10.0f, 20.0f, 30.0f});
    Vec3f p = t.transform_point({0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(p.x, 10.0f, 1e-5f);
    ASSERT_NEAR(p.y, 20.0f, 1e-5f);
    ASSERT_NEAR(p.z, 30.0f, 1e-5f);
}

TEST_CASE(mat4_scale) {
    Mat4 s = Mat4::scale({2.0f, 3.0f, 4.0f});
    Vec3f p = s.transform_point({1.0f, 1.0f, 1.0f});
    ASSERT_NEAR(p.x, 2.0f, 1e-5f);
    ASSERT_NEAR(p.y, 3.0f, 1e-5f);
    ASSERT_NEAR(p.z, 4.0f, 1e-5f);
}

TEST_CASE(mat4_multiply_identity) {
    Mat4 a;
    Mat4 b;
    Mat4 c = a * b;
    ASSERT_NEAR(c(0, 0), 1.0f, 1e-6f);
    ASSERT_NEAR(c(1, 1), 1.0f, 1e-6f);
    ASSERT_NEAR(c(2, 2), 1.0f, 1e-6f);
    ASSERT_NEAR(c(0, 1), 0.0f, 1e-6f);
}

TEST_CASE(mat4_translation_composition) {
    Mat4 t1 = Mat4::translation({1.0f, 0.0f, 0.0f});
    Mat4 t2 = Mat4::translation({0.0f, 2.0f, 0.0f});
    Mat4 t = t1 * t2;
    Vec3f p = t.transform_point({0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(p.x, 1.0f, 1e-5f);
    ASSERT_NEAR(p.y, 2.0f, 1e-5f);
}

TEST_CASE(mat4_transform_direction) {
    Mat4 t = Mat4::translation({100.0f, 200.0f, 300.0f});
    Vec3f d = t.transform_direction({1.0f, 0.0f, 0.0f});
    // Direction should not be affected by translation
    ASSERT_NEAR(d.x, 1.0f, 1e-5f);
    ASSERT_NEAR(d.y, 0.0f, 1e-5f);
    ASSERT_NEAR(d.z, 0.0f, 1e-5f);
}
