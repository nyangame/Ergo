#include "test_framework.hpp"
#include "engine/physics/collision3d.hpp"

TEST_CASE(Collision3D_SphereSphere_Hit) {
    SphereShape s1{1.0f};
    SphereShape s2{1.0f};
    Transform3D t1; t1.position = {0.0f, 0.0f, 0.0f};
    Transform3D t2; t2.position = {1.5f, 0.0f, 0.0f};

    auto contact = collide_sphere_sphere(s1, t1, s2, t2);
    ASSERT_TRUE(contact.has_value());
    ASSERT_TRUE(contact->penetration > 0.0f);
}

TEST_CASE(Collision3D_SphereSphere_NoHit) {
    SphereShape s1{1.0f};
    SphereShape s2{1.0f};
    Transform3D t1; t1.position = {0.0f, 0.0f, 0.0f};
    Transform3D t2; t2.position = {3.0f, 0.0f, 0.0f};

    auto contact = collide_sphere_sphere(s1, t1, s2, t2);
    ASSERT_FALSE(contact.has_value());
}

TEST_CASE(Collision3D_SpherePlane_Hit) {
    SphereShape sphere{1.0f};
    PlaneShape plane; plane.normal = {0, 1, 0}; plane.offset = 0;
    Transform3D ts; ts.position = {0.0f, 0.5f, 0.0f};

    auto contact = collide_sphere_plane(sphere, ts, plane);
    ASSERT_TRUE(contact.has_value());
    ASSERT_TRUE(contact->penetration > 0.0f);
}

TEST_CASE(Collision3D_SpherePlane_NoHit) {
    SphereShape sphere{1.0f};
    PlaneShape plane; plane.normal = {0, 1, 0}; plane.offset = 0;
    Transform3D ts; ts.position = {0.0f, 2.0f, 0.0f};

    auto contact = collide_sphere_plane(sphere, ts, plane);
    ASSERT_FALSE(contact.has_value());
}

TEST_CASE(Collision3D_Generic_SphereSphere) {
    CollisionShape3D a = SphereShape{1.0f};
    CollisionShape3D b = SphereShape{1.0f};
    Transform3D ta; ta.position = {0, 0, 0};
    Transform3D tb; tb.position = {1, 0, 0};

    auto contact = check_collision3d(a, ta, b, tb);
    ASSERT_TRUE(contact.has_value());
}

TEST_CASE(Collision3D_Generic_SpherePlane) {
    CollisionShape3D a = SphereShape{1.0f};
    CollisionShape3D b = PlaneShape{{0, 1, 0}, 0.0f};
    Transform3D ta; ta.position = {0, 0.5f, 0};
    Transform3D tb;

    auto contact = check_collision3d(a, ta, b, tb);
    ASSERT_TRUE(contact.has_value());
}
