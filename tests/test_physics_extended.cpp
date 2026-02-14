#include "framework/test_framework.hpp"
#include "engine/physics/spatial_grid.hpp"
#include "engine/physics/collision3d.hpp"
#include "engine/physics/rigid_body.hpp"
#include "engine/physics/rigid_body_world.hpp"

using namespace ergo::test;

// ============================================================
// Physics/SpatialGrid2D
// ============================================================

static TestSuite spatial_grid_suite("Physics/SpatialGrid2D");

static void register_spatial_grid_tests() {
    spatial_grid_suite.add("SpatialGrid_CellSize", [](TestContext& ctx) {
        SpatialGrid2D grid(128.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, grid.cell_size(), 128.0f, 0.001f);
    });

    spatial_grid_suite.add("SpatialGrid_InsertAndQuery", [](TestContext& ctx) {
        SpatialGrid2D grid(64.0f);

        Transform2D t1; t1.position = {10.0f, 10.0f};
        Collider c1; c1.handle = {1}; c1.transform = &t1;
        c1.shape = AABBData{Vec2f{5.0f, 5.0f}};

        grid.insert(&c1);

        auto results = grid.query({0.0f, 0.0f}, {64.0f, 64.0f});
        ERGO_TEST_ASSERT_EQ(ctx, results.size(), (size_t)1);
    });

    spatial_grid_suite.add("SpatialGrid_QueryOutOfRange", [](TestContext& ctx) {
        SpatialGrid2D grid(64.0f);

        Transform2D t1; t1.position = {10.0f, 10.0f};
        Collider c1; c1.handle = {1}; c1.transform = &t1;
        c1.shape = AABBData{Vec2f{5.0f, 5.0f}};

        grid.insert(&c1);

        auto results = grid.query({500.0f, 500.0f}, {600.0f, 600.0f});
        ERGO_TEST_ASSERT_EQ(ctx, results.size(), (size_t)0);
    });

    spatial_grid_suite.add("SpatialGrid_MultipleColliders", [](TestContext& ctx) {
        SpatialGrid2D grid(64.0f);

        Transform2D t1; t1.position = {10.0f, 10.0f};
        Transform2D t2; t2.position = {20.0f, 20.0f};
        Transform2D t3; t3.position = {500.0f, 500.0f};

        Collider c1; c1.handle = {1}; c1.transform = &t1;
        c1.shape = AABBData{Vec2f{5.0f, 5.0f}};
        Collider c2; c2.handle = {2}; c2.transform = &t2;
        c2.shape = CircleData{5.0f};
        Collider c3; c3.handle = {3}; c3.transform = &t3;
        c3.shape = AABBData{Vec2f{5.0f, 5.0f}};

        grid.insert(&c1);
        grid.insert(&c2);
        grid.insert(&c3);

        auto near = grid.query({0.0f, 0.0f}, {64.0f, 64.0f});
        ERGO_TEST_ASSERT_EQ(ctx, near.size(), (size_t)2);

        auto far = grid.query({450.0f, 450.0f}, {550.0f, 550.0f});
        ERGO_TEST_ASSERT_EQ(ctx, far.size(), (size_t)1);
    });

    spatial_grid_suite.add("SpatialGrid_Clear", [](TestContext& ctx) {
        SpatialGrid2D grid(64.0f);

        Transform2D t1; t1.position = {10.0f, 10.0f};
        Collider c1; c1.handle = {1}; c1.transform = &t1;
        c1.shape = AABBData{Vec2f{5.0f, 5.0f}};
        grid.insert(&c1);

        grid.clear();
        auto results = grid.query({0.0f, 0.0f}, {64.0f, 64.0f});
        ERGO_TEST_ASSERT_EQ(ctx, results.size(), (size_t)0);
    });
}

// ============================================================
// Physics/Collision3D
// ============================================================

static TestSuite collision3d_suite("Physics/Collision3D");

static void register_collision3d_tests() {
    collision3d_suite.add("Collision3D_SphereSphere_Hit", [](TestContext& ctx) {
        SphereShape s1{1.0f};
        SphereShape s2{1.0f};
        Transform3D t1; t1.position = {0.0f, 0.0f, 0.0f};
        Transform3D t2; t2.position = {1.5f, 0.0f, 0.0f};

        auto contact = collide_sphere_sphere(s1, t1, s2, t2);
        ERGO_TEST_ASSERT_TRUE(ctx, contact.has_value());
        ERGO_TEST_ASSERT_TRUE(ctx, contact->penetration > 0.0f);
    });

    collision3d_suite.add("Collision3D_SphereSphere_NoHit", [](TestContext& ctx) {
        SphereShape s1{1.0f};
        SphereShape s2{1.0f};
        Transform3D t1; t1.position = {0.0f, 0.0f, 0.0f};
        Transform3D t2; t2.position = {3.0f, 0.0f, 0.0f};

        auto contact = collide_sphere_sphere(s1, t1, s2, t2);
        ERGO_TEST_ASSERT_FALSE(ctx, contact.has_value());
    });

    collision3d_suite.add("Collision3D_SpherePlane_Hit", [](TestContext& ctx) {
        SphereShape sphere{1.0f};
        PlaneShape plane; plane.normal = {0, 1, 0}; plane.offset = 0;
        Transform3D ts; ts.position = {0.0f, 0.5f, 0.0f};

        auto contact = collide_sphere_plane(sphere, ts, plane);
        ERGO_TEST_ASSERT_TRUE(ctx, contact.has_value());
        ERGO_TEST_ASSERT_TRUE(ctx, contact->penetration > 0.0f);
    });

    collision3d_suite.add("Collision3D_SpherePlane_NoHit", [](TestContext& ctx) {
        SphereShape sphere{1.0f};
        PlaneShape plane; plane.normal = {0, 1, 0}; plane.offset = 0;
        Transform3D ts; ts.position = {0.0f, 2.0f, 0.0f};

        auto contact = collide_sphere_plane(sphere, ts, plane);
        ERGO_TEST_ASSERT_FALSE(ctx, contact.has_value());
    });

    collision3d_suite.add("Collision3D_Generic_SphereSphere", [](TestContext& ctx) {
        CollisionShape3D a = SphereShape{1.0f};
        CollisionShape3D b = SphereShape{1.0f};
        Transform3D ta; ta.position = {0, 0, 0};
        Transform3D tb; tb.position = {1, 0, 0};

        auto contact = check_collision3d(a, ta, b, tb);
        ERGO_TEST_ASSERT_TRUE(ctx, contact.has_value());
    });

    collision3d_suite.add("Collision3D_Generic_SpherePlane", [](TestContext& ctx) {
        CollisionShape3D a = SphereShape{1.0f};
        CollisionShape3D b = PlaneShape{{0, 1, 0}, 0.0f};
        Transform3D ta; ta.position = {0, 0.5f, 0};
        Transform3D tb;

        auto contact = check_collision3d(a, ta, b, tb);
        ERGO_TEST_ASSERT_TRUE(ctx, contact.has_value());
    });
}

// ============================================================
// Physics/RigidBody
// ============================================================

static TestSuite rigid_body_suite("Physics/RigidBody");

static void register_rigid_body_tests() {
    rigid_body_suite.add("RigidBody_SetMass", [](TestContext& ctx) {
        RigidBody body;
        body.set_mass(2.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, body.mass, 2.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, body.inv_mass, 0.5f, 0.001f);
    });

    rigid_body_suite.add("RigidBody_SetStatic", [](TestContext& ctx) {
        RigidBody body;
        body.set_static();
        ERGO_TEST_ASSERT_NEAR(ctx, body.inv_mass, 0.0f, 0.001f);
        ERGO_TEST_ASSERT_TRUE(ctx, body.type == RigidBodyType::Static);
    });

    rigid_body_suite.add("RigidBody_ApplyForce", [](TestContext& ctx) {
        RigidBody body;
        body.set_mass(1.0f);
        body.apply_force({10.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, body.force_accumulator.x, 10.0f, 0.001f);
    });

    rigid_body_suite.add("RigidBody_ApplyForce_Static", [](TestContext& ctx) {
        RigidBody body;
        body.set_static();
        body.apply_force({10.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, body.force_accumulator.x, 0.0f, 0.001f);
    });

    rigid_body_suite.add("RigidBody_ApplyImpulse", [](TestContext& ctx) {
        RigidBody body;
        body.set_mass(2.0f);
        body.apply_impulse({10.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, body.velocity.x, 5.0f, 0.001f);  // impulse * inv_mass
    });

    rigid_body_suite.add("RigidBody_ClearForces", [](TestContext& ctx) {
        RigidBody body;
        body.set_mass(1.0f);
        body.apply_force({10.0f, 5.0f, 3.0f});
        body.apply_torque({1.0f, 2.0f, 3.0f});
        body.clear_forces();
        ERGO_TEST_ASSERT_NEAR(ctx, body.force_accumulator.x, 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, body.torque_accumulator.x, 0.0f, 0.001f);
    });

    rigid_body_suite.add("RigidBody_Sleep", [](TestContext& ctx) {
        RigidBody body;
        body.is_sleeping = true;
        body.set_mass(1.0f);
        body.apply_force({1.0f, 0.0f, 0.0f});
        ERGO_TEST_ASSERT_FALSE(ctx, body.is_sleeping);  // wake on force
    });

    rigid_body_suite.add("RigidBodyWorld_AddRemove", [](TestContext& ctx) {
        RigidBodyWorld world;

        PhysicsBody pb;
        pb.body.set_mass(1.0f);
        pb.shape = SphereShape{1.0f};
        auto id = world.add_body(pb);

        ERGO_TEST_ASSERT_EQ(ctx, world.body_count(), (size_t)1);
        ERGO_TEST_ASSERT_TRUE(ctx, world.get_body(id) != nullptr);

        world.remove_body(id);
        ERGO_TEST_ASSERT_EQ(ctx, world.body_count(), (size_t)0);
    });

    rigid_body_suite.add("RigidBodyWorld_Gravity", [](TestContext& ctx) {
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
        ERGO_TEST_ASSERT_TRUE(ctx, b != nullptr);
        // Ball should have moved downward
        ERGO_TEST_ASSERT_TRUE(ctx, b->body.velocity.y < 0.0f);
    });
}

// ============================================================
// Registration entry point
// ============================================================

void register_physics_extended_tests(TestRunner& runner) {
    register_spatial_grid_tests();
    register_collision3d_tests();
    register_rigid_body_tests();

    runner.add_suite(spatial_grid_suite);
    runner.add_suite(collision3d_suite);
    runner.add_suite(rigid_body_suite);
}
