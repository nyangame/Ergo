#include "test_framework.hpp"
#include "engine/physics/rigid_body.hpp"
#include "engine/physics/rigid_body_world.hpp"

TEST_CASE(RigidBody_SetMass) {
    RigidBody body;
    body.set_mass(2.0f);
    ASSERT_NEAR(body.mass, 2.0f, 0.001f);
    ASSERT_NEAR(body.inv_mass, 0.5f, 0.001f);
}

TEST_CASE(RigidBody_SetStatic) {
    RigidBody body;
    body.set_static();
    ASSERT_NEAR(body.inv_mass, 0.0f, 0.001f);
    ASSERT_TRUE(body.type == RigidBodyType::Static);
}

TEST_CASE(RigidBody_ApplyForce) {
    RigidBody body;
    body.set_mass(1.0f);
    body.apply_force({10.0f, 0.0f, 0.0f});
    ASSERT_NEAR(body.force_accumulator.x, 10.0f, 0.001f);
}

TEST_CASE(RigidBody_ApplyForce_Static) {
    RigidBody body;
    body.set_static();
    body.apply_force({10.0f, 0.0f, 0.0f});
    ASSERT_NEAR(body.force_accumulator.x, 0.0f, 0.001f);
}

TEST_CASE(RigidBody_ApplyImpulse) {
    RigidBody body;
    body.set_mass(2.0f);
    body.apply_impulse({10.0f, 0.0f, 0.0f});
    ASSERT_NEAR(body.velocity.x, 5.0f, 0.001f);  // impulse * inv_mass
}

TEST_CASE(RigidBody_ClearForces) {
    RigidBody body;
    body.set_mass(1.0f);
    body.apply_force({10.0f, 5.0f, 3.0f});
    body.apply_torque({1.0f, 2.0f, 3.0f});
    body.clear_forces();
    ASSERT_NEAR(body.force_accumulator.x, 0.0f, 0.001f);
    ASSERT_NEAR(body.torque_accumulator.x, 0.0f, 0.001f);
}

TEST_CASE(RigidBody_Sleep) {
    RigidBody body;
    body.is_sleeping = true;
    body.set_mass(1.0f);
    body.apply_force({1.0f, 0.0f, 0.0f});
    ASSERT_FALSE(body.is_sleeping);  // wake on force
}

TEST_CASE(RigidBodyWorld_AddRemove) {
    RigidBodyWorld world;

    PhysicsBody pb;
    pb.body.set_mass(1.0f);
    pb.shape = SphereShape{1.0f};
    auto id = world.add_body(pb);

    ASSERT_EQ(world.body_count(), (size_t)1);
    ASSERT_TRUE(world.get_body(id) != nullptr);

    world.remove_body(id);
    ASSERT_EQ(world.body_count(), (size_t)0);
}

TEST_CASE(RigidBodyWorld_Gravity) {
    RigidBodyWorld world;
    world.set_gravity({0.0f, -10.0f, 0.0f});

    PhysicsBody ball;
    ball.body.set_mass(1.0f);
    ball.body.type = RigidBodyType::Dynamic;
    ball.shape = SphereShape{0.5f};
    ball.transform.position = {0.0f, 10.0f, 0.0f};
    ball.body.transform = &ball.transform;
    auto id = world.add_body(ball);

    // Step several times
    for (int i = 0; i < 10; ++i) {
        world.step(1.0f / 60.0f);
    }

    auto* b = world.get_body(id);
    ASSERT_TRUE(b != nullptr);
    // Ball should have moved downward
    ASSERT_TRUE(b->body.velocity.y < 0.0f);
}
