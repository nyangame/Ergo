#include "demo_framework.hpp"
#include "engine/core/camera2d.hpp"
#include "engine/core/camera3d.hpp"
#include <cstdio>

DEMO(Camera2D_Projection) {
    Camera2D cam;
    cam.position = {100.0f, 200.0f};
    cam.zoom = 2.0f;
    cam.viewport_width = 800.0f;
    cam.viewport_height = 600.0f;

    std::printf("  Camera pos=(%.1f, %.1f) zoom=%.1f\n",
                cam.position.x, cam.position.y, cam.zoom);

    Vec2f screen = cam.world_to_screen({100.0f, 200.0f});
    std::printf("  World (100,200) -> Screen (%.1f, %.1f)\n", screen.x, screen.y);

    Vec2f world = cam.screen_to_world({400.0f, 300.0f});
    std::printf("  Screen (400,300) -> World (%.1f, %.1f)\n", world.x, world.y);

    Mat4 vp = cam.view_projection();
    std::printf("  ViewProjection m[0]=%.6f m[5]=%.6f\n", vp.m[0], vp.m[5]);
}

DEMO(Camera2D_Shake) {
    Camera2D cam;
    cam.position = {0.0f, 0.0f};

    cam.shake(10.0f, 0.5f);
    std::printf("  Shake started: intensity=10, duration=0.5\n");

    for (int i = 0; i < 5; ++i) {
        cam.update_shake(0.1f);
        Vec2f screen = cam.world_to_screen({0.0f, 0.0f});
        std::printf("    Frame %d: screen center=(%.2f, %.2f)\n", i, screen.x, screen.y);
    }
}

DEMO(Camera3D_LookAt) {
    Camera3D cam;
    cam.position = {0.0f, 5.0f, 10.0f};
    cam.target = {0.0f, 0.0f, 0.0f};
    cam.fov = 60.0f;
    cam.aspect = 16.0f / 9.0f;

    std::printf("  Camera pos=(%.1f, %.1f, %.1f) target=(%.1f, %.1f, %.1f)\n",
                cam.position.x, cam.position.y, cam.position.z,
                cam.target.x, cam.target.y, cam.target.z);

    Vec3f fwd = cam.forward();
    std::printf("  Forward: (%.4f, %.4f, %.4f)\n", fwd.x, fwd.y, fwd.z);

    Vec3f right = cam.right_dir();
    std::printf("  Right:   (%.4f, %.4f, %.4f)\n", right.x, right.y, right.z);

    Mat4 vp = cam.view_projection();
    std::printf("  VP m[0]=%.6f m[5]=%.6f\n", vp.m[0], vp.m[5]);
}
