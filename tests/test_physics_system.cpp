#include "test_framework.hpp"
#include "engine/physics/physics_system.hpp"

TEST_CASE(physics_register_collider) {
    PhysicsSystem physics;
    Transform2D t{{0.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};
    Collider c;
    c.shape = AABBData{{10.0f, 10.0f}};
    c.tag = ColliderTag::Player;
    c.transform = &t;

    auto handle = physics.register_collider(c);
    ASSERT_TRUE(handle.valid());
}

TEST_CASE(physics_collision_callback) {
    PhysicsSystem physics;

    Transform2D t1{{0.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};
    Transform2D t2{{5.0f, 0.0f}, 0.0f, {20.0f, 20.0f}};

    bool hit_detected = false;

    Collider c1;
    c1.shape = AABBData{{10.0f, 10.0f}};
    c1.tag = ColliderTag::Player;
    c1.transform = &t1;
    c1.on_hit = [&](const Collider&) { hit_detected = true; return false; };

    Collider c2;
    c2.shape = AABBData{{10.0f, 10.0f}};
    c2.tag = ColliderTag::Enemy;
    c2.transform = &t2;

    physics.register_collider(c1);
    physics.register_collider(c2);
    physics.mark_moved(c1);  // Must mark as moved to trigger collision check
    physics.run();

    ASSERT_TRUE(hit_detected);
}
