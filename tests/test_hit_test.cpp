#include "test_framework.hpp"
#include "engine/physics/hit_test.hpp"

TEST_CASE(aabb_overlap) {
    Transform2D t1{{0.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};
    Transform2D t2{{15.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};

    Collider c1;
    c1.shape = AABBData{{10.0f, 10.0f}};
    c1.transform = &t1;

    Collider c2;
    c2.shape = AABBData{{10.0f, 10.0f}};
    c2.transform = &t2;

    ASSERT_TRUE(check_hit(c1, c2));
}

TEST_CASE(aabb_no_overlap) {
    Transform2D t1{{0.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};
    Transform2D t2{{25.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};

    Collider c1;
    c1.shape = AABBData{{10.0f, 10.0f}};
    c1.transform = &t1;

    Collider c2;
    c2.shape = AABBData{{10.0f, 10.0f}};
    c2.transform = &t2;

    ASSERT_FALSE(check_hit(c1, c2));
}

TEST_CASE(circle_overlap) {
    Transform2D t1{{0.0f, 0.0f}, 0.0f, {10.0f, 10.0f}};
    Transform2D t2{{8.0f, 0.0f}, 0.0f, {10.0f, 10.0f}};

    Collider c1;
    c1.shape = CircleData{5.0f};
    c1.transform = &t1;

    Collider c2;
    c2.shape = CircleData{5.0f};
    c2.transform = &t2;

    ASSERT_TRUE(check_hit(c1, c2));
}

TEST_CASE(circle_no_overlap) {
    Transform2D t1{{0.0f, 0.0f}, 0.0f, {10.0f, 10.0f}};
    Transform2D t2{{20.0f, 0.0f}, 0.0f, {10.0f, 10.0f}};

    Collider c1;
    c1.shape = CircleData{5.0f};
    c1.transform = &t1;

    Collider c2;
    c2.shape = CircleData{5.0f};
    c2.transform = &t2;

    ASSERT_FALSE(check_hit(c1, c2));
}

TEST_CASE(circle_aabb_overlap) {
    Transform2D t1{{0.0f, 0.0f}, 0.0f, {10.0f, 10.0f}};
    Transform2D t2{{8.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};

    Collider c1;
    c1.shape = CircleData{5.0f};
    c1.transform = &t1;

    Collider c2;
    c2.shape = AABBData{{10.0f, 10.0f}};
    c2.transform = &t2;

    ASSERT_TRUE(check_hit(c1, c2));
}
