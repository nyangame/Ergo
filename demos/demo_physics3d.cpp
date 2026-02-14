#include "demo_framework.hpp"
#include "engine/physics/collision3d.hpp"
#include "engine/physics/rigid_body.hpp"
#include "engine/physics/rigid_body_world.hpp"
#include <cstdio>

DEMO(Physics3D_SphereCollision) {
    SphereShape s1{1.0f};
    SphereShape s2{1.0f};

    Transform3D t1; t1.position = {0.0f, 0.0f, 0.0f};
    Transform3D t2; t2.position = {1.5f, 0.0f, 0.0f};

    auto contact = collide_sphere_sphere(s1, t1, s2, t2);
    if (contact) {
        std::printf("  Sphere-Sphere collision: point=(%.2f,%.2f,%.2f) pen=%.4f\n",
                    contact->point.x, contact->point.y, contact->point.z,
                    contact->penetration);
    } else {
        std::printf("  Sphere-Sphere: no collision\n");
    }
}

DEMO(Physics3D_SpherePlane) {
    SphereShape sphere{1.0f};
    PlaneShape ground;
    ground.normal = {0.0f, 1.0f, 0.0f};
    ground.offset = 0.0f;

    Transform3D ts; ts.position = {0.0f, 0.5f, 0.0f};

    auto contact = collide_sphere_plane(sphere, ts, ground);
    if (contact) {
        std::printf("  Sphere(y=0.5) vs Ground: pen=%.4f normal=(%.1f,%.1f,%.1f)\n",
                    contact->penetration, contact->normal.x, contact->normal.y, contact->normal.z);
    }
}

DEMO(Physics3D_RigidBody) {
    RigidBody body;
    body.set_mass(2.0f);
    std::printf("  Mass: %.1f, InvMass: %.4f\n", body.mass, body.inv_mass);

    body.apply_force({0.0f, -9.81f * body.mass, 0.0f});
    std::printf("  Force accumulator: (%.2f, %.2f, %.2f)\n",
                body.force_accumulator.x, body.force_accumulator.y, body.force_accumulator.z);

    body.apply_impulse({5.0f, 0.0f, 0.0f});
    std::printf("  Velocity after impulse: (%.4f, %.4f, %.4f)\n",
                body.velocity.x, body.velocity.y, body.velocity.z);

    body.clear_forces();
    std::printf("  After clear_forces: (%.2f, %.2f, %.2f)\n",
                body.force_accumulator.x, body.force_accumulator.y, body.force_accumulator.z);
}

DEMO(Physics3D_RigidBodyWorld) {
    RigidBodyWorld world;
    world.set_gravity({0.0f, -9.81f, 0.0f});

    // Add a dynamic sphere
    PhysicsBody ball;
    ball.body.set_mass(1.0f);
    ball.body.type = RigidBodyType::Dynamic;
    ball.shape = SphereShape{0.5f};
    ball.transform.position = {0.0f, 5.0f, 0.0f};
    ball.body.transform = &ball.transform;
    auto ball_id = world.add_body(ball);

    // Add a static ground plane
    PhysicsBody ground;
    ground.body.set_static();
    ground.shape = PlaneShape{{0.0f, 1.0f, 0.0f}, 0.0f};
    ground.transform.position = {0.0f, 0.0f, 0.0f};
    ground.body.transform = &ground.transform;
    world.add_body(ground);

    std::printf("  Bodies: %zu, gravity=(%.2f, %.2f, %.2f)\n",
                world.body_count(), world.gravity().x, world.gravity().y, world.gravity().z);

    // Simulate 10 steps
    for (int i = 0; i < 10; ++i) {
        world.step(1.0f / 60.0f);
    }

    auto* b = world.get_body(ball_id);
    if (b) {
        std::printf("  Ball after 10 steps: y=%.4f vy=%.4f\n",
                    b->transform.position.y, b->body.velocity.y);
    }
}
