#include "test_framework.hpp"
#include "engine/math/vec2.hpp"

TEST_CASE(vec2_default_construction) {
    Vec2f v;
    ASSERT_NEAR(v.x, 0.0f, 1e-6f);
    ASSERT_NEAR(v.y, 0.0f, 1e-6f);
}

TEST_CASE(vec2_value_construction) {
    Vec2f v(3.0f, 4.0f);
    ASSERT_NEAR(v.x, 3.0f, 1e-6f);
    ASSERT_NEAR(v.y, 4.0f, 1e-6f);
}

TEST_CASE(vec2_addition) {
    Vec2f a(1.0f, 2.0f);
    Vec2f b(3.0f, 4.0f);
    Vec2f c = a + b;
    ASSERT_NEAR(c.x, 4.0f, 1e-6f);
    ASSERT_NEAR(c.y, 6.0f, 1e-6f);
}

TEST_CASE(vec2_subtraction) {
    Vec2f a(5.0f, 7.0f);
    Vec2f b(2.0f, 3.0f);
    Vec2f c = a - b;
    ASSERT_NEAR(c.x, 3.0f, 1e-6f);
    ASSERT_NEAR(c.y, 4.0f, 1e-6f);
}

TEST_CASE(vec2_scalar_multiply) {
    Vec2f v(3.0f, 4.0f);
    Vec2f r = v * 2.0f;
    ASSERT_NEAR(r.x, 6.0f, 1e-6f);
    ASSERT_NEAR(r.y, 8.0f, 1e-6f);
}

TEST_CASE(vec2_length) {
    Vec2f v(3.0f, 4.0f);
    ASSERT_NEAR(v.length(), 5.0f, 1e-5f);
    ASSERT_NEAR(v.length_sq(), 25.0f, 1e-5f);
}

TEST_CASE(vec2_normalize) {
    Vec2f v(3.0f, 4.0f);
    Vec2f n = v.normalized();
    ASSERT_NEAR(n.length(), 1.0f, 1e-5f);
    ASSERT_NEAR(n.x, 0.6f, 1e-5f);
    ASSERT_NEAR(n.y, 0.8f, 1e-5f);
}

TEST_CASE(vec2_normalize_zero) {
    Vec2f v(0.0f, 0.0f);
    Vec2f n = v.normalized();
    ASSERT_NEAR(n.x, 0.0f, 1e-6f);
    ASSERT_NEAR(n.y, 0.0f, 1e-6f);
}

TEST_CASE(vec2_compound_assignment) {
    Vec2f v(1.0f, 2.0f);
    v += Vec2f(3.0f, 4.0f);
    ASSERT_NEAR(v.x, 4.0f, 1e-6f);
    ASSERT_NEAR(v.y, 6.0f, 1e-6f);

    v -= Vec2f(1.0f, 1.0f);
    ASSERT_NEAR(v.x, 3.0f, 1e-6f);
    ASSERT_NEAR(v.y, 5.0f, 1e-6f);

    v *= 2.0f;
    ASSERT_NEAR(v.x, 6.0f, 1e-6f);
    ASSERT_NEAR(v.y, 10.0f, 1e-6f);
}

TEST_CASE(vec2_zero) {
    Vec2f v = Vec2f::zero();
    ASSERT_NEAR(v.x, 0.0f, 1e-6f);
    ASSERT_NEAR(v.y, 0.0f, 1e-6f);
}
