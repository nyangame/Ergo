#include "framework/test_framework.hpp"
#include "engine/math/vec2.hpp"
#include "engine/math/vec3.hpp"
#include "engine/math/mat4.hpp"
#include "engine/math/quat.hpp"
#include "engine/math/size2.hpp"
#include "engine/math/color.hpp"
#include "engine/math/transform.hpp"
#include "engine/math/transform3d.hpp"

using namespace ergo::test;

static constexpr float kEps = 1e-5f;

// ============================================================
// Vec2f tests
// ============================================================

static TestSuite suite_vec2("Math/Vec2f");

static void register_vec2_tests() {
    suite_vec2.add("default_constructor", [](TestContext& ctx) {
        constexpr Vec2f v;
        ERGO_TEST_ASSERT_EQ(ctx, v.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.y, 0.0f);
    });

    suite_vec2.add("value_constructor", [](TestContext& ctx) {
        constexpr Vec2f v(3.0f, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.x, 3.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.y, 4.0f);
    });

    suite_vec2.add("add", [](TestContext& ctx) {
        constexpr Vec2f a(1.0f, 2.0f);
        constexpr Vec2f b(3.0f, 4.0f);
        constexpr Vec2f c = a + b;
        ERGO_TEST_ASSERT_EQ(ctx, c.x, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.y, 6.0f);
    });

    suite_vec2.add("subtract", [](TestContext& ctx) {
        constexpr Vec2f a(5.0f, 7.0f);
        constexpr Vec2f b(2.0f, 3.0f);
        constexpr Vec2f c = a - b;
        ERGO_TEST_ASSERT_EQ(ctx, c.x, 3.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.y, 4.0f);
    });

    suite_vec2.add("multiply_scalar", [](TestContext& ctx) {
        constexpr Vec2f a(2.0f, 3.0f);
        constexpr Vec2f b = a * 2.0f;
        ERGO_TEST_ASSERT_EQ(ctx, b.x, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, b.y, 6.0f);
    });

    suite_vec2.add("compound_add", [](TestContext& ctx) {
        Vec2f a(1.0f, 2.0f);
        a += Vec2f(3.0f, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, a.x, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, a.y, 6.0f);
    });

    suite_vec2.add("compound_sub", [](TestContext& ctx) {
        Vec2f a(5.0f, 7.0f);
        a -= Vec2f(2.0f, 3.0f);
        ERGO_TEST_ASSERT_EQ(ctx, a.x, 3.0f);
        ERGO_TEST_ASSERT_EQ(ctx, a.y, 4.0f);
    });

    suite_vec2.add("compound_mul", [](TestContext& ctx) {
        Vec2f a(2.0f, 3.0f);
        a *= 3.0f;
        ERGO_TEST_ASSERT_EQ(ctx, a.x, 6.0f);
        ERGO_TEST_ASSERT_EQ(ctx, a.y, 9.0f);
    });

    suite_vec2.add("length_sq", [](TestContext& ctx) {
        constexpr Vec2f v(3.0f, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.length_sq(), 25.0f);
    });

    suite_vec2.add("length", [](TestContext& ctx) {
        Vec2f v(3.0f, 4.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, v.length(), 5.0f, kEps);
    });

    suite_vec2.add("normalized", [](TestContext& ctx) {
        Vec2f v(3.0f, 4.0f);
        Vec2f n = v.normalized();
        ERGO_TEST_ASSERT_NEAR(ctx, n.x, 0.6f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, n.y, 0.8f, kEps);
    });

    suite_vec2.add("normalized_zero_vector", [](TestContext& ctx) {
        Vec2f v(0.0f, 0.0f);
        Vec2f n = v.normalized();
        ERGO_TEST_ASSERT_EQ(ctx, n.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, n.y, 0.0f);
    });

    suite_vec2.add("zero_static", [](TestContext& ctx) {
        constexpr Vec2f z = Vec2f::zero();
        ERGO_TEST_ASSERT_EQ(ctx, z.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, z.y, 0.0f);
    });
}

// ============================================================
// Vec3f tests
// ============================================================

static TestSuite suite_vec3("Math/Vec3f");

static void register_vec3_tests() {
    suite_vec3.add("default_constructor", [](TestContext& ctx) {
        constexpr Vec3f v;
        ERGO_TEST_ASSERT_EQ(ctx, v.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.y, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.z, 0.0f);
    });

    suite_vec3.add("value_constructor", [](TestContext& ctx) {
        constexpr Vec3f v(1.0f, 2.0f, 3.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.x, 1.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.y, 2.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.z, 3.0f);
    });

    suite_vec3.add("add", [](TestContext& ctx) {
        constexpr Vec3f a(1.0f, 2.0f, 3.0f);
        constexpr Vec3f b(4.0f, 5.0f, 6.0f);
        constexpr Vec3f c = a + b;
        ERGO_TEST_ASSERT_EQ(ctx, c.x, 5.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.y, 7.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.z, 9.0f);
    });

    suite_vec3.add("subtract", [](TestContext& ctx) {
        constexpr Vec3f a(5.0f, 7.0f, 9.0f);
        constexpr Vec3f b(1.0f, 2.0f, 3.0f);
        constexpr Vec3f c = a - b;
        ERGO_TEST_ASSERT_EQ(ctx, c.x, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.y, 5.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.z, 6.0f);
    });

    suite_vec3.add("multiply_scalar", [](TestContext& ctx) {
        constexpr Vec3f a(1.0f, 2.0f, 3.0f);
        constexpr Vec3f b = a * 2.0f;
        ERGO_TEST_ASSERT_EQ(ctx, b.x, 2.0f);
        ERGO_TEST_ASSERT_EQ(ctx, b.y, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, b.z, 6.0f);
    });

    suite_vec3.add("divide_scalar", [](TestContext& ctx) {
        constexpr Vec3f a(6.0f, 8.0f, 10.0f);
        constexpr Vec3f b = a / 2.0f;
        ERGO_TEST_ASSERT_EQ(ctx, b.x, 3.0f);
        ERGO_TEST_ASSERT_EQ(ctx, b.y, 4.0f);
        ERGO_TEST_ASSERT_EQ(ctx, b.z, 5.0f);
    });

    suite_vec3.add("dot_product", [](TestContext& ctx) {
        constexpr Vec3f a(1.0f, 2.0f, 3.0f);
        constexpr Vec3f b(4.0f, 5.0f, 6.0f);
        constexpr float d = a.dot(b);
        ERGO_TEST_ASSERT_EQ(ctx, d, 32.0f);  // 1*4 + 2*5 + 3*6
    });

    suite_vec3.add("cross_product", [](TestContext& ctx) {
        constexpr Vec3f a(1.0f, 0.0f, 0.0f);
        constexpr Vec3f b(0.0f, 1.0f, 0.0f);
        constexpr Vec3f c = a.cross(b);
        ERGO_TEST_ASSERT_EQ(ctx, c.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.y, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.z, 1.0f);  // x cross y = z
    });

    suite_vec3.add("cross_product_anticommutative", [](TestContext& ctx) {
        constexpr Vec3f a(1.0f, 2.0f, 3.0f);
        constexpr Vec3f b(4.0f, 5.0f, 6.0f);
        constexpr Vec3f ab = a.cross(b);
        constexpr Vec3f ba = b.cross(a);
        ERGO_TEST_ASSERT_NEAR(ctx, ab.x, -ba.x, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, ab.y, -ba.y, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, ab.z, -ba.z, kEps);
    });

    suite_vec3.add("length_sq", [](TestContext& ctx) {
        constexpr Vec3f v(1.0f, 2.0f, 2.0f);
        ERGO_TEST_ASSERT_EQ(ctx, v.length_sq(), 9.0f);
    });

    suite_vec3.add("length", [](TestContext& ctx) {
        Vec3f v(1.0f, 2.0f, 2.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, v.length(), 3.0f, kEps);
    });

    suite_vec3.add("normalized", [](TestContext& ctx) {
        Vec3f v(0.0f, 3.0f, 0.0f);
        Vec3f n = v.normalized();
        ERGO_TEST_ASSERT_NEAR(ctx, n.x, 0.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, n.y, 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, n.z, 0.0f, kEps);
    });

    suite_vec3.add("static_constants", [](TestContext& ctx) {
        constexpr Vec3f z = Vec3f::zero();
        ERGO_TEST_ASSERT_EQ(ctx, z.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, z.y, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, z.z, 0.0f);

        constexpr Vec3f u = Vec3f::up();
        ERGO_TEST_ASSERT_EQ(ctx, u.y, 1.0f);

        constexpr Vec3f f = Vec3f::forward();
        ERGO_TEST_ASSERT_EQ(ctx, f.z, -1.0f);

        constexpr Vec3f r = Vec3f::right();
        ERGO_TEST_ASSERT_EQ(ctx, r.x, 1.0f);
    });
}

// ============================================================
// Mat4 tests
// ============================================================

static TestSuite suite_mat4("Math/Mat4");

static void register_mat4_tests() {
    suite_mat4.add("identity", [](TestContext& ctx) {
        constexpr Mat4 m;
        ERGO_TEST_ASSERT_EQ(ctx, m(0, 0), 1.0f);
        ERGO_TEST_ASSERT_EQ(ctx, m(1, 1), 1.0f);
        ERGO_TEST_ASSERT_EQ(ctx, m(2, 2), 1.0f);
        ERGO_TEST_ASSERT_EQ(ctx, m(3, 3), 1.0f);
        ERGO_TEST_ASSERT_EQ(ctx, m(0, 1), 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, m(1, 0), 0.0f);
    });

    suite_mat4.add("translation", [](TestContext& ctx) {
        constexpr Mat4 t = Mat4::translation({10.0f, 20.0f, 30.0f});
        constexpr Vec3f p = t.transform_point(Vec3f::zero());
        ERGO_TEST_ASSERT_NEAR(ctx, p.x, 10.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.y, 20.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.z, 30.0f, kEps);
    });

    suite_mat4.add("scale", [](TestContext& ctx) {
        constexpr Mat4 s = Mat4::scale({2.0f, 3.0f, 4.0f});
        constexpr Vec3f p = s.transform_point({1.0f, 1.0f, 1.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, p.x, 2.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.y, 3.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.z, 4.0f, kEps);
    });

    suite_mat4.add("multiply_identity", [](TestContext& ctx) {
        constexpr Mat4 a = Mat4::translation({5.0f, 0.0f, 0.0f});
        constexpr Mat4 id;
        constexpr Mat4 r = a * id;
        constexpr Vec3f p = r.transform_point(Vec3f::zero());
        ERGO_TEST_ASSERT_NEAR(ctx, p.x, 5.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.y, 0.0f, kEps);
    });

    suite_mat4.add("transform_direction_ignores_translation", [](TestContext& ctx) {
        constexpr Mat4 t = Mat4::translation({100.0f, 200.0f, 300.0f});
        constexpr Vec3f d = t.transform_direction({1.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, d.x, 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, d.y, 0.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, d.z, 0.0f, kEps);
    });

    suite_mat4.add("rotation_y_90deg", [](TestContext& ctx) {
        constexpr float pi = 3.14159265358979f;
        Mat4 r = Mat4::rotation_y(pi / 2.0f);
        Vec3f p = r.transform_point({1.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, p.x, 0.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.z, -1.0f, kEps);
    });

    suite_mat4.add("translation_then_scale", [](TestContext& ctx) {
        constexpr Mat4 t = Mat4::translation({1.0f, 0.0f, 0.0f});
        constexpr Mat4 s = Mat4::scale({2.0f, 2.0f, 2.0f});
        constexpr Mat4 ts = t * s;
        constexpr Vec3f p = ts.transform_point({1.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, p.x, 3.0f, kEps);  // scale(1*2) + translate(1)
    });

    suite_mat4.add("perspective_valid", [](TestContext& ctx) {
        constexpr float pi = 3.14159265358979f;
        Mat4 proj = Mat4::perspective(pi / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
        // Diagonal elements should be non-zero
        ERGO_TEST_ASSERT(ctx, proj(0, 0) != 0.0f);
        ERGO_TEST_ASSERT(ctx, proj(1, 1) != 0.0f);
        ERGO_TEST_ASSERT(ctx, proj(2, 2) != 0.0f);
    });
}

// ============================================================
// Quat tests
// ============================================================

static TestSuite suite_quat("Math/Quat");

static void register_quat_tests() {
    suite_quat.add("identity", [](TestContext& ctx) {
        constexpr Quat q = Quat::identity();
        ERGO_TEST_ASSERT_EQ(ctx, q.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, q.y, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, q.z, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, q.w, 1.0f);
    });

    suite_quat.add("identity_rotation_preserves_vector", [](TestContext& ctx) {
        constexpr Quat q = Quat::identity();
        constexpr Vec3f v(1.0f, 2.0f, 3.0f);
        constexpr Vec3f r = q.rotate(v);
        ERGO_TEST_ASSERT_NEAR(ctx, r.x, 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r.y, 2.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r.z, 3.0f, kEps);
    });

    suite_quat.add("rotation_90deg_y_axis", [](TestContext& ctx) {
        constexpr float pi = 3.14159265358979f;
        Quat q = Quat::from_axis_angle(Vec3f::up(), pi / 2.0f);
        Vec3f r = q.rotate({1.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, r.x, 0.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r.y, 0.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r.z, -1.0f, kEps);
    });

    suite_quat.add("conjugate", [](TestContext& ctx) {
        constexpr Quat q(1.0f, 2.0f, 3.0f, 4.0f);
        constexpr Quat c = q.conjugate();
        ERGO_TEST_ASSERT_EQ(ctx, c.x, -1.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.y, -2.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.z, -3.0f);
        ERGO_TEST_ASSERT_EQ(ctx, c.w, 4.0f);
    });

    suite_quat.add("length_unit_quaternion", [](TestContext& ctx) {
        Quat q = Quat::from_axis_angle(Vec3f::up(), 1.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, q.length(), 1.0f, kEps);
    });

    suite_quat.add("normalized", [](TestContext& ctx) {
        Quat q(1.0f, 2.0f, 3.0f, 4.0f);
        Quat n = q.normalized();
        ERGO_TEST_ASSERT_NEAR(ctx, n.length(), 1.0f, kEps);
    });

    suite_quat.add("multiply_identity", [](TestContext& ctx) {
        constexpr float pi = 3.14159265358979f;
        Quat q = Quat::from_axis_angle(Vec3f::up(), pi / 3.0f);
        constexpr Quat id = Quat::identity();
        Quat r = q * id;
        ERGO_TEST_ASSERT_NEAR(ctx, r.x, q.x, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r.y, q.y, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r.z, q.z, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r.w, q.w, kEps);
    });

    suite_quat.add("to_mat4_identity", [](TestContext& ctx) {
        constexpr Quat q = Quat::identity();
        Mat4 m = q.to_mat4();
        ERGO_TEST_ASSERT_NEAR(ctx, m(0, 0), 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, m(1, 1), 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, m(2, 2), 1.0f, kEps);
    });

    suite_quat.add("slerp_endpoints", [](TestContext& ctx) {
        constexpr float pi = 3.14159265358979f;
        Quat a = Quat::identity();
        Quat b = Quat::from_axis_angle(Vec3f::up(), pi / 2.0f);

        Quat r0 = Quat::slerp(a, b, 0.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, r0.x, a.x, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r0.w, a.w, kEps);

        Quat r1 = Quat::slerp(a, b, 1.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, r1.x, b.x, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r1.y, b.y, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, r1.w, b.w, kEps);
    });
}

// ============================================================
// Size2f / Color / Transform tests
// ============================================================

static TestSuite suite_misc_math("Math/Misc");

static void register_misc_math_tests() {
    suite_misc_math.add("Size2f_defaults", [](TestContext& ctx) {
        constexpr Size2f s;
        ERGO_TEST_ASSERT_EQ(ctx, s.w, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, s.h, 0.0f);
    });

    suite_misc_math.add("Size2f_half", [](TestContext& ctx) {
        constexpr Size2f s(100.0f, 60.0f);
        ERGO_TEST_ASSERT_EQ(ctx, s.half_w(), 50.0f);
        ERGO_TEST_ASSERT_EQ(ctx, s.half_h(), 30.0f);
        ERGO_TEST_ASSERT_EQ(ctx, s.radius(), 50.0f);
    });

    suite_misc_math.add("Color_default_white", [](TestContext& ctx) {
        constexpr Color c;
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.r, 255);
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.g, 255);
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.b, 255);
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.a, 255);
    });

    suite_misc_math.add("Color_custom", [](TestContext& ctx) {
        constexpr Color c(128, 64, 32, 200);
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.r, 128);
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.g, 64);
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.b, 32);
        ERGO_TEST_ASSERT_EQ(ctx, (int)c.a, 200);
    });

    suite_misc_math.add("Transform2D_defaults", [](TestContext& ctx) {
        Transform2D t;
        ERGO_TEST_ASSERT_EQ(ctx, t.position.x, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, t.position.y, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, t.rotation, 0.0f);
    });

    suite_misc_math.add("Transform3D_to_mat4_identity", [](TestContext& ctx) {
        Transform3D t;
        t.position = Vec3f::zero();
        t.rotation = Quat::identity();
        t.scale_ = {1.0f, 1.0f, 1.0f};
        Mat4 m = t.to_mat4();
        ERGO_TEST_ASSERT_NEAR(ctx, m(0, 0), 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, m(1, 1), 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, m(2, 2), 1.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, m(3, 3), 1.0f, kEps);
    });

    suite_misc_math.add("Transform3D_translation_in_mat4", [](TestContext& ctx) {
        Transform3D t;
        t.position = {5.0f, 10.0f, 15.0f};
        t.rotation = Quat::identity();
        t.scale_ = {1.0f, 1.0f, 1.0f};
        Mat4 m = t.to_mat4();
        Vec3f p = m.transform_point(Vec3f::zero());
        ERGO_TEST_ASSERT_NEAR(ctx, p.x, 5.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.y, 10.0f, kEps);
        ERGO_TEST_ASSERT_NEAR(ctx, p.z, 15.0f, kEps);
    });
}

// ============================================================
// Registration
// ============================================================

void register_math_tests(TestRunner& runner) {
    register_vec2_tests();
    register_vec3_tests();
    register_mat4_tests();
    register_quat_tests();
    register_misc_math_tests();

    runner.add_suite(suite_vec2);
    runner.add_suite(suite_vec3);
    runner.add_suite(suite_mat4);
    runner.add_suite(suite_quat);
    runner.add_suite(suite_misc_math);
}
