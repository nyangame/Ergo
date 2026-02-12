#include "test_framework.hpp"
#include "engine/core/camera2d.hpp"
#include "engine/core/camera3d.hpp"

TEST_CASE(Camera2D_WorldToScreen_Center) {
    Camera2D cam;
    cam.position = {0.0f, 0.0f};
    cam.zoom = 1.0f;
    cam.viewport_width = 800.0f;
    cam.viewport_height = 600.0f;

    Vec2f screen = cam.world_to_screen({0.0f, 0.0f});
    ASSERT_NEAR(screen.x, 400.0f, 0.01f);
    ASSERT_NEAR(screen.y, 300.0f, 0.01f);
}

TEST_CASE(Camera2D_ScreenToWorld_Roundtrip) {
    Camera2D cam;
    cam.position = {50.0f, 100.0f};
    cam.zoom = 2.0f;
    cam.viewport_width = 800.0f;
    cam.viewport_height = 600.0f;

    Vec2f world = {75.0f, 120.0f};
    Vec2f screen = cam.world_to_screen(world);
    Vec2f back = cam.screen_to_world(screen);
    ASSERT_NEAR(back.x, world.x, 0.01f);
    ASSERT_NEAR(back.y, world.y, 0.01f);
}

TEST_CASE(Camera2D_Zoom) {
    Camera2D cam;
    cam.position = {0.0f, 0.0f};
    cam.viewport_width = 800.0f;
    cam.viewport_height = 600.0f;

    cam.zoom = 1.0f;
    Vec2f s1 = cam.world_to_screen({100.0f, 0.0f});

    cam.zoom = 2.0f;
    Vec2f s2 = cam.world_to_screen({100.0f, 0.0f});

    // At zoom=2, world point should be further from center on screen
    ASSERT_TRUE(std::abs(s2.x - 400.0f) > std::abs(s1.x - 400.0f));
}

TEST_CASE(Camera3D_Forward) {
    Camera3D cam;
    cam.position = {0.0f, 0.0f, 10.0f};
    cam.target = {0.0f, 0.0f, 0.0f};

    Vec3f fwd = cam.forward();
    ASSERT_NEAR(fwd.x, 0.0f, 0.01f);
    ASSERT_NEAR(fwd.y, 0.0f, 0.01f);
    ASSERT_TRUE(fwd.z < 0.0f);  // pointing -Z
}

TEST_CASE(Camera3D_ViewProjection) {
    Camera3D cam;
    cam.position = {0.0f, 5.0f, 10.0f};
    cam.target = {0.0f, 0.0f, 0.0f};
    cam.fov = 60.0f;
    cam.aspect = 16.0f / 9.0f;

    Mat4 vp = cam.view_projection();
    // VP should be non-identity
    ASSERT_TRUE(std::abs(vp.m[0] - 1.0f) > 0.001f || std::abs(vp.m[5] - 1.0f) > 0.001f);
}
