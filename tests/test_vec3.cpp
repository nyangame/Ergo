#include "test_framework.hpp"
#include "engine/math/vec3.hpp"

TEST_CASE(vec3_default_construction) {
    Vec3f v;
    ASSERT_NEAR(v.x, 0.0f, 1e-6f);
    ASSERT_NEAR(v.y, 0.0f, 1e-6f);
    ASSERT_NEAR(v.z, 0.0f, 1e-6f);
}

TEST_CASE(vec3_operations) {
    Vec3f a(1.0f, 2.0f, 3.0f);
    Vec3f b(4.0f, 5.0f, 6.0f);
    Vec3f c = a + b;
    ASSERT_NEAR(c.x, 5.0f, 1e-6f);
    ASSERT_NEAR(c.y, 7.0f, 1e-6f);
    ASSERT_NEAR(c.z, 9.0f, 1e-6f);
}

TEST_CASE(vec3_dot_product) {
    Vec3f a(1.0f, 0.0f, 0.0f);
    Vec3f b(0.0f, 1.0f, 0.0f);
    ASSERT_NEAR(a.dot(b), 0.0f, 1e-6f);

    Vec3f c(1.0f, 2.0f, 3.0f);
    Vec3f d(4.0f, 5.0f, 6.0f);
    ASSERT_NEAR(c.dot(d), 32.0f, 1e-5f);
}

TEST_CASE(vec3_cross_product) {
    Vec3f x(1.0f, 0.0f, 0.0f);
    Vec3f y(0.0f, 1.0f, 0.0f);
    Vec3f z = x.cross(y);
    ASSERT_NEAR(z.x, 0.0f, 1e-6f);
    ASSERT_NEAR(z.y, 0.0f, 1e-6f);
    ASSERT_NEAR(z.z, 1.0f, 1e-6f);
}

TEST_CASE(vec3_length) {
    Vec3f v(1.0f, 2.0f, 2.0f);
    ASSERT_NEAR(v.length(), 3.0f, 1e-5f);
}

TEST_CASE(vec3_normalize) {
    Vec3f v(3.0f, 0.0f, 4.0f);
    Vec3f n = v.normalized();
    ASSERT_NEAR(n.length(), 1.0f, 1e-5f);
}

TEST_CASE(vec3_static_directions) {
    Vec3f up = Vec3f::up();
    ASSERT_NEAR(up.y, 1.0f, 1e-6f);
    Vec3f fwd = Vec3f::forward();
    ASSERT_NEAR(fwd.z, -1.0f, 1e-6f);
    Vec3f right = Vec3f::right();
    ASSERT_NEAR(right.x, 1.0f, 1e-6f);
}
