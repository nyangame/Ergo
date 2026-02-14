#include "framework/test_framework.hpp"
#include "engine/physics/hit_test.hpp"

using namespace ergo::test;

// ============================================================
// AABB collision tests
// ============================================================

static TestSuite suite_aabb("Physics/AABB");

static void register_aabb_tests() {
    suite_aabb.add("overlapping_boxes", [](TestContext& ctx) {
        AABBData a{Vec2f(1.0f, 1.0f)};
        AABBData b{Vec2f(1.0f, 1.0f)};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {1.0f, 1.0f};
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(a, ta, b, tb));
    });

    suite_aabb.add("separated_boxes_x", [](TestContext& ctx) {
        AABBData a{Vec2f(1.0f, 1.0f)};
        AABBData b{Vec2f(1.0f, 1.0f)};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {3.0f, 0.0f};
        ERGO_TEST_ASSERT_FALSE(ctx, hit_test(a, ta, b, tb));
    });

    suite_aabb.add("separated_boxes_y", [](TestContext& ctx) {
        AABBData a{Vec2f(1.0f, 1.0f)};
        AABBData b{Vec2f(1.0f, 1.0f)};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {0.0f, 3.0f};
        ERGO_TEST_ASSERT_FALSE(ctx, hit_test(a, ta, b, tb));
    });

    suite_aabb.add("touching_edge", [](TestContext& ctx) {
        AABBData a{Vec2f(1.0f, 1.0f)};
        AABBData b{Vec2f(1.0f, 1.0f)};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {2.0f, 0.0f};
        // edge-touching: xa2=1, xb1=1 -> 1 >= 1 = true
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(a, ta, b, tb));
    });

    suite_aabb.add("same_position", [](TestContext& ctx) {
        AABBData a{Vec2f(2.0f, 2.0f)};
        AABBData b{Vec2f(1.0f, 1.0f)};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {0.0f, 0.0f};
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(a, ta, b, tb));
    });

    suite_aabb.add("asymmetric_sizes", [](TestContext& ctx) {
        AABBData a{Vec2f(10.0f, 0.5f)};  // wide but thin
        AABBData b{Vec2f(0.5f, 10.0f)};  // thin but tall
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {0.0f, 0.0f};
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(a, ta, b, tb));
    });
}

// ============================================================
// Circle collision tests
// ============================================================

static TestSuite suite_circle("Physics/Circle");

static void register_circle_tests() {
    suite_circle.add("overlapping_circles", [](TestContext& ctx) {
        CircleData a{2.0f};
        CircleData b{2.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {3.0f, 0.0f};
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(a, ta, b, tb));
    });

    suite_circle.add("separated_circles", [](TestContext& ctx) {
        CircleData a{1.0f};
        CircleData b{1.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {3.0f, 0.0f};
        ERGO_TEST_ASSERT_FALSE(ctx, hit_test(a, ta, b, tb));
    });

    suite_circle.add("same_position", [](TestContext& ctx) {
        CircleData a{1.0f};
        CircleData b{1.0f};
        Transform2D ta; ta.position = {5.0f, 5.0f};
        Transform2D tb; tb.position = {5.0f, 5.0f};
        // distance=0 < (1+1)^2=4 -> hit
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(a, ta, b, tb));
    });

    suite_circle.add("just_outside", [](TestContext& ctx) {
        CircleData a{1.0f};
        CircleData b{1.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {2.1f, 0.0f};
        // distance_sq = 4.41, r_sum_sq = 4.0 -> no hit
        ERGO_TEST_ASSERT_FALSE(ctx, hit_test(a, ta, b, tb));
    });

    suite_circle.add("diagonal_overlap", [](TestContext& ctx) {
        CircleData a{2.0f};
        CircleData b{2.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {2.0f, 2.0f};
        // distance_sq = 8, r_sum_sq = 16 -> hit
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(a, ta, b, tb));
    });
}

// ============================================================
// Circle vs AABB collision tests
// ============================================================

static TestSuite suite_circle_aabb("Physics/CircleVsAABB");

static void register_circle_aabb_tests() {
    suite_circle_aabb.add("circle_inside_aabb", [](TestContext& ctx) {
        CircleData circle{0.5f};
        AABBData aabb{Vec2f(5.0f, 5.0f)};
        Transform2D tc; tc.position = {0.0f, 0.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(circle, tc, aabb, ta));
    });

    suite_circle_aabb.add("circle_outside_aabb", [](TestContext& ctx) {
        CircleData circle{1.0f};
        AABBData aabb{Vec2f(1.0f, 1.0f)};
        Transform2D tc; tc.position = {5.0f, 5.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        ERGO_TEST_ASSERT_FALSE(ctx, hit_test(circle, tc, aabb, ta));
    });

    suite_circle_aabb.add("circle_touching_aabb_side", [](TestContext& ctx) {
        CircleData circle{1.0f};
        AABBData aabb{Vec2f(2.0f, 2.0f)};
        Transform2D tc; tc.position = {2.5f, 0.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        // circle center at 2.5, radius 1.0, box right edge at 2.0
        // Extended rect check: x1-r=-3 < 2.5 < x2+r=3, y1=- 2 < 0 < y2=2 -> hit
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(circle, tc, aabb, ta));
    });

    suite_circle_aabb.add("circle_near_corner", [](TestContext& ctx) {
        CircleData circle{1.5f};
        AABBData aabb{Vec2f(1.0f, 1.0f)};
        Transform2D tc; tc.position = {2.0f, 2.0f};
        Transform2D ta; ta.position = {0.0f, 0.0f};
        // Corner at (1,1), distance to (2,2) = sqrt(2) ~= 1.414 < 1.5 -> hit
        ERGO_TEST_ASSERT_TRUE(ctx, hit_test(circle, tc, aabb, ta));
    });

    suite_circle_aabb.add("aabb_vs_circle_commutative", [](TestContext& ctx) {
        CircleData circle{2.0f};
        AABBData aabb{Vec2f(1.0f, 1.0f)};
        Transform2D tc; tc.position = {0.0f, 0.0f};
        Transform2D ta; ta.position = {2.0f, 0.0f};
        bool r1 = hit_test(circle, tc, aabb, ta);
        bool r2 = hit_test(aabb, ta, circle, tc);
        ERGO_TEST_ASSERT_EQ(ctx, r1, r2);
    });
}

// ============================================================
// Variant-based check_hit tests
// ============================================================

static TestSuite suite_check_hit("Physics/CheckHit");

static void register_check_hit_tests() {
    suite_check_hit.add("variant_aabb_vs_aabb", [](TestContext& ctx) {
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {1.0f, 0.0f};
        Collider a; a.shape = AABBData{Vec2f(1.0f, 1.0f)}; a.transform = &ta;
        Collider b; b.shape = AABBData{Vec2f(1.0f, 1.0f)}; b.transform = &tb;
        ERGO_TEST_ASSERT_TRUE(ctx, check_hit(a, b));
    });

    suite_check_hit.add("variant_circle_vs_circle", [](TestContext& ctx) {
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {10.0f, 0.0f};
        Collider a; a.shape = CircleData{1.0f}; a.transform = &ta;
        Collider b; b.shape = CircleData{1.0f}; b.transform = &tb;
        ERGO_TEST_ASSERT_FALSE(ctx, check_hit(a, b));
    });

    suite_check_hit.add("variant_mixed_types", [](TestContext& ctx) {
        Transform2D ta; ta.position = {0.0f, 0.0f};
        Transform2D tb; tb.position = {0.0f, 0.0f};
        Collider a; a.shape = CircleData{5.0f}; a.transform = &ta;
        Collider b; b.shape = AABBData{Vec2f(1.0f, 1.0f)}; b.transform = &tb;
        ERGO_TEST_ASSERT_TRUE(ctx, check_hit(a, b));
    });
}

// ============================================================
// Registration
// ============================================================

void register_physics_tests(TestRunner& runner) {
    register_aabb_tests();
    register_circle_tests();
    register_circle_aabb_tests();
    register_check_hit_tests();

    runner.add_suite(suite_aabb);
    runner.add_suite(suite_circle);
    runner.add_suite(suite_circle_aabb);
    runner.add_suite(suite_check_hit);
}
