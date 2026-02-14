#include "test_framework.hpp"
#include "engine/math/quat.hpp"

TEST_CASE(quat_identity) {
    Quat q = Quat::identity();
    ASSERT_NEAR(q.x, 0.0f, 1e-6f);
    ASSERT_NEAR(q.y, 0.0f, 1e-6f);
    ASSERT_NEAR(q.z, 0.0f, 1e-6f);
    ASSERT_NEAR(q.w, 1.0f, 1e-6f);
}

TEST_CASE(quat_identity_rotation) {
    Quat q = Quat::identity();
    Vec3f v = q.rotate({1.0f, 0.0f, 0.0f});
    ASSERT_NEAR(v.x, 1.0f, 1e-5f);
    ASSERT_NEAR(v.y, 0.0f, 1e-5f);
    ASSERT_NEAR(v.z, 0.0f, 1e-5f);
}

TEST_CASE(quat_90_degree_rotation) {
    // 90 degrees around Y axis
    float angle = 3.14159265f * 0.5f;
    Quat q = Quat::from_axis_angle({0.0f, 1.0f, 0.0f}, angle);
    Vec3f v = q.rotate({1.0f, 0.0f, 0.0f});
    // (1,0,0) rotated 90 around Y -> (0,0,-1)
    ASSERT_NEAR(v.x, 0.0f, 1e-4f);
    ASSERT_NEAR(v.y, 0.0f, 1e-4f);
    ASSERT_NEAR(v.z, -1.0f, 1e-4f);
}

TEST_CASE(quat_normalize) {
    Quat q(1.0f, 2.0f, 3.0f, 4.0f);
    Quat n = q.normalized();
    ASSERT_NEAR(n.length(), 1.0f, 1e-5f);
}

TEST_CASE(quat_conjugate) {
    Quat q(1.0f, 2.0f, 3.0f, 4.0f);
    Quat c = q.conjugate();
    ASSERT_NEAR(c.x, -1.0f, 1e-6f);
    ASSERT_NEAR(c.y, -2.0f, 1e-6f);
    ASSERT_NEAR(c.z, -3.0f, 1e-6f);
    ASSERT_NEAR(c.w, 4.0f, 1e-6f);
}

TEST_CASE(quat_slerp_endpoints) {
    Quat a = Quat::identity();
    Quat b = Quat::from_axis_angle({0.0f, 1.0f, 0.0f}, 1.0f);

    Quat r0 = Quat::slerp(a, b, 0.0f);
    ASSERT_NEAR(r0.x, a.x, 1e-5f);
    ASSERT_NEAR(r0.w, a.w, 1e-5f);

    Quat r1 = Quat::slerp(a, b, 1.0f);
    ASSERT_NEAR(r1.x, b.x, 1e-5f);
    ASSERT_NEAR(r1.w, b.w, 1e-5f);
}

TEST_CASE(quat_multiplication) {
    Quat a = Quat::identity();
    Quat b = Quat::identity();
    Quat c = a * b;
    ASSERT_NEAR(c.w, 1.0f, 1e-5f);
    ASSERT_NEAR(c.x, 0.0f, 1e-5f);
}
