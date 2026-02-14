#include "framework/test_framework.hpp"
#include "engine/core/camera2d.hpp"
#include "engine/core/camera3d.hpp"
#include "engine/core/easing.hpp"
#include "engine/core/sprite_animation.hpp"
#include "engine/core/input_map.hpp"
#include "engine/core/time.hpp"

using namespace ergo::test;

// ============================================================
// Camera tests
// ============================================================

static TestSuite suite_camera("Gameplay/Camera");

static void register_camera_tests() {
    suite_camera.add("Camera2D_WorldToScreen_Center", [](TestContext& ctx) {
        Camera2D cam;
        cam.position = {0.0f, 0.0f};
        cam.zoom = 1.0f;
        cam.viewport_width = 800.0f;
        cam.viewport_height = 600.0f;

        Vec2f screen = cam.world_to_screen({0.0f, 0.0f});
        ERGO_TEST_ASSERT_NEAR(ctx, screen.x, 400.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, screen.y, 300.0f, 0.01f);
    });

    suite_camera.add("Camera2D_ScreenToWorld_Roundtrip", [](TestContext& ctx) {
        Camera2D cam;
        cam.position = {50.0f, 100.0f};
        cam.zoom = 2.0f;
        cam.viewport_width = 800.0f;
        cam.viewport_height = 600.0f;

        Vec2f world = {75.0f, 120.0f};
        Vec2f screen = cam.world_to_screen(world);
        Vec2f back = cam.screen_to_world(screen);
        ERGO_TEST_ASSERT_NEAR(ctx, back.x, world.x, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, back.y, world.y, 0.01f);
    });

    suite_camera.add("Camera2D_Zoom", [](TestContext& ctx) {
        Camera2D cam;
        cam.position = {0.0f, 0.0f};
        cam.viewport_width = 800.0f;
        cam.viewport_height = 600.0f;

        cam.zoom = 1.0f;
        Vec2f s1 = cam.world_to_screen({100.0f, 0.0f});

        cam.zoom = 2.0f;
        Vec2f s2 = cam.world_to_screen({100.0f, 0.0f});

        // At zoom=2, world point should be further from center on screen
        ERGO_TEST_ASSERT_TRUE(ctx, std::abs(s2.x - 400.0f) > std::abs(s1.x - 400.0f));
    });

    suite_camera.add("Camera3D_Forward", [](TestContext& ctx) {
        Camera3D cam;
        cam.position = {0.0f, 0.0f, 10.0f};
        cam.target = {0.0f, 0.0f, 0.0f};

        Vec3f fwd = cam.forward();
        ERGO_TEST_ASSERT_NEAR(ctx, fwd.x, 0.0f, 0.01f);
        ERGO_TEST_ASSERT_NEAR(ctx, fwd.y, 0.0f, 0.01f);
        ERGO_TEST_ASSERT_TRUE(ctx, fwd.z < 0.0f);  // pointing -Z
    });

    suite_camera.add("Camera3D_ViewProjection", [](TestContext& ctx) {
        Camera3D cam;
        cam.position = {0.0f, 5.0f, 10.0f};
        cam.target = {0.0f, 0.0f, 0.0f};
        cam.fov = 60.0f;
        cam.aspect = 16.0f / 9.0f;

        Mat4 vp = cam.view_projection();
        // VP should be non-identity
        ERGO_TEST_ASSERT_TRUE(ctx, std::abs(vp.m[0] - 1.0f) > 0.001f || std::abs(vp.m[5] - 1.0f) > 0.001f);
    });
}

// ============================================================
// Easing tests
// ============================================================

static TestSuite suite_easing("Gameplay/Easing");

static void register_easing_tests() {
    suite_easing.add("Easing_Linear", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::linear(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::linear(0.5f), 0.5f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::linear(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_InQuad_Endpoints", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_quad(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_quad(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_OutQuad_Endpoints", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_quad(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_quad(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_InOutQuad_Midpoint", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_out_quad(0.5f), 0.5f, 0.001f);
    });

    suite_easing.add("Easing_InCubic", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_cubic(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_cubic(1.0f), 1.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_cubic(0.5f), 0.125f, 0.001f);
    });

    suite_easing.add("Easing_OutCubic", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_cubic(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_cubic(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_Sine_Endpoints", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_sine(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_sine(1.0f), 1.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_sine(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_sine(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_Expo_Endpoints", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_expo(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_expo(1.0f), 1.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_expo(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_expo(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_Elastic_Endpoints", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_elastic(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_elastic(1.0f), 1.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_elastic(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_elastic(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_Bounce_Endpoints", [](TestContext& ctx) {
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_bounce(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::out_bounce(1.0f), 1.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_bounce(0.0f), 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, easing::in_bounce(1.0f), 1.0f, 0.001f);
    });

    suite_easing.add("Easing_Back_Overshoots", [](TestContext& ctx) {
        // in_back should go negative before 0.5
        float early = easing::in_back(0.1f);
        ERGO_TEST_ASSERT_TRUE(ctx, early < 0.0f);

        // out_back should overshoot past 1.0
        float late = easing::out_back(0.9f);
        ERGO_TEST_ASSERT_TRUE(ctx, late > 1.0f);
    });
}

// ============================================================
// SpriteAnimation tests
// ============================================================

static TestSuite suite_sprite_animation("Gameplay/SpriteAnimation");

static void register_sprite_animation_tests() {
    suite_sprite_animation.add("SpriteAnimation_FromGrid", [](TestContext& ctx) {
        auto anim = SpriteAnimation::from_grid(TextureHandle{1}, 4, 2, 8, 0.1f);
        ERGO_TEST_ASSERT_EQ(ctx, anim.frames.size(), (size_t)8);
        ERGO_TEST_ASSERT_NEAR(ctx, anim.frames[0].uv.x, 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, anim.frames[0].uv.w, 0.25f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, anim.frames[0].uv.h, 0.5f, 0.001f);
    });

    suite_sprite_animation.add("SpriteAnimation_FrameAdvance", [](TestContext& ctx) {
        SpriteAnimation anim;
        anim.loop = true;
        anim.frames = {
            {Rect{0, 0, 1, 1}, 0.1f},
            {Rect{0, 0, 1, 1}, 0.1f},
            {Rect{0, 0, 1, 1}, 0.1f},
        };

        ERGO_TEST_ASSERT_EQ(ctx, anim.current_frame, (uint32_t)0);
        anim.update(0.15f);
        ERGO_TEST_ASSERT_EQ(ctx, anim.current_frame, (uint32_t)1);
    });

    suite_sprite_animation.add("SpriteAnimation_Loop", [](TestContext& ctx) {
        SpriteAnimation anim;
        anim.loop = true;
        anim.frames = {
            {Rect{0, 0, 1, 1}, 0.1f},
            {Rect{0, 0, 1, 1}, 0.1f},
        };

        anim.update(0.25f);  // Past all frames
        ERGO_TEST_ASSERT_FALSE(ctx, anim.finished);
        ERGO_TEST_ASSERT_EQ(ctx, anim.current_frame, (uint32_t)0);
    });

    suite_sprite_animation.add("SpriteAnimation_NoLoop", [](TestContext& ctx) {
        SpriteAnimation anim;
        anim.loop = false;
        anim.frames = {
            {Rect{0, 0, 1, 1}, 0.1f},
            {Rect{0, 0, 1, 1}, 0.1f},
        };

        anim.update(0.25f);
        ERGO_TEST_ASSERT_TRUE(ctx, anim.finished);
        ERGO_TEST_ASSERT_EQ(ctx, anim.current_frame, (uint32_t)1);
    });

    suite_sprite_animation.add("SpriteAnimation_Reset", [](TestContext& ctx) {
        SpriteAnimation anim;
        anim.loop = false;
        anim.frames = {
            {Rect{0, 0, 1, 1}, 0.1f},
            {Rect{0, 0, 1, 1}, 0.1f},
        };

        anim.update(0.25f);
        ERGO_TEST_ASSERT_TRUE(ctx, anim.finished);

        anim.reset();
        ERGO_TEST_ASSERT_FALSE(ctx, anim.finished);
        ERGO_TEST_ASSERT_EQ(ctx, anim.current_frame, (uint32_t)0);
        ERGO_TEST_ASSERT_NEAR(ctx, anim.timer, 0.0f, 0.001f);
    });

    suite_sprite_animation.add("AnimationController_Play", [](TestContext& ctx) {
        AnimationController ctrl;

        SpriteAnimation idle;
        idle.frames = {{Rect{0, 0, 1, 1}, 0.5f}};
        idle.loop = true;

        SpriteAnimation run;
        run.frames = {{Rect{0, 0, 0.25f, 1}, 0.1f}, {Rect{0.25f, 0, 0.25f, 1}, 0.1f}};
        run.loop = true;

        ctrl.animations["idle"] = idle;
        ctrl.animations["run"] = run;

        ctrl.play("idle");
        ERGO_TEST_ASSERT_TRUE(ctx, ctrl.current_name == "idle");
        ERGO_TEST_ASSERT_TRUE(ctx, ctrl.current() != nullptr);

        ctrl.play("run");
        ERGO_TEST_ASSERT_TRUE(ctx, ctrl.current_name == "run");

        // Playing same animation again should not reset
        ctrl.update(0.05f);
        ctrl.play("run");
        ERGO_TEST_ASSERT_TRUE(ctx, ctrl.current() != nullptr);
    });
}

// ============================================================
// InputMap tests
// ============================================================

static TestSuite suite_input_map("Gameplay/InputMap");

static void register_input_map_tests() {
    suite_input_map.add("InputMap_RegisterAction", [](TestContext& ctx) {
        InputMap imap;
        InputAction jump;
        jump.name = "jump";
        jump.keys = {32};
        imap.register_action(jump);

        auto* action = imap.get_action("jump");
        ERGO_TEST_ASSERT_TRUE(ctx, action != nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, action->name == "jump");
    });

    suite_input_map.add("InputMap_IsActionDown", [](TestContext& ctx) {
        InputMap imap;
        InputAction fire;
        fire.name = "fire";
        fire.keys = {90};
        imap.register_action(fire);

        imap.set_key_state(90, true);
        ERGO_TEST_ASSERT_TRUE(ctx, imap.is_action_down("fire"));

        imap.set_key_state(90, false);
        ERGO_TEST_ASSERT_FALSE(ctx, imap.is_action_down("fire"));
    });

    suite_input_map.add("InputMap_IsActionPressed", [](TestContext& ctx) {
        InputMap imap;
        InputAction jump;
        jump.name = "jump";
        jump.keys = {32};
        imap.register_action(jump);

        // Key was not down previous frame, now it is
        imap.set_previous_key_state(32, false);
        imap.set_key_state(32, true);
        ERGO_TEST_ASSERT_TRUE(ctx, imap.is_action_pressed("jump"));

        // Key was already down previous frame
        imap.set_previous_key_state(32, true);
        imap.set_key_state(32, true);
        ERGO_TEST_ASSERT_FALSE(ctx, imap.is_action_pressed("jump"));
    });

    suite_input_map.add("InputMap_GamepadAxis", [](TestContext& ctx) {
        InputMap imap;
        InputAction move;
        move.name = "move_x";
        move.gamepad_axis = 0;
        move.dead_zone = 0.15f;
        imap.register_action(move);

        imap.set_gamepad_axis(0, 0.8f);
        ERGO_TEST_ASSERT_NEAR(ctx, imap.get_axis("move_x"), 0.8f, 0.001f);

        imap.set_gamepad_axis(0, 0.1f);
        ERGO_TEST_ASSERT_NEAR(ctx, imap.get_axis("move_x"), 0.0f, 0.001f);  // Below dead zone
    });

    suite_input_map.add("InputMap_UnregisterAction", [](TestContext& ctx) {
        InputMap imap;
        InputAction fire;
        fire.name = "fire";
        fire.keys = {90};
        imap.register_action(fire);

        imap.unregister_action("fire");
        auto* action = imap.get_action("fire");
        ERGO_TEST_ASSERT_TRUE(ctx, action == nullptr);
    });

    suite_input_map.add("InputMap_NonexistentAction", [](TestContext& ctx) {
        InputMap imap;
        ERGO_TEST_ASSERT_FALSE(ctx, imap.is_action_down("nonexistent"));
        ERGO_TEST_ASSERT_FALSE(ctx, imap.is_action_pressed("nonexistent"));
    });
}

// ============================================================
// Time tests
// ============================================================

static TestSuite suite_time("Gameplay/Time");

static void register_time_tests() {
    suite_time.add("Time_Reset", [](TestContext& ctx) {
        Time t;
        t.tick(0.1f);
        t.reset();
        ERGO_TEST_ASSERT_NEAR(ctx, t.delta_time, 0.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, t.total_time, 0.0f, 0.001f);
        ERGO_TEST_ASSERT_EQ(ctx, t.frame_count, (uint64_t)0);
    });

    suite_time.add("Time_Tick", [](TestContext& ctx) {
        Time t;
        t.reset();
        t.tick(1.0f / 60.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, t.delta_time, 1.0f / 60.0f, 0.0001f);
        ERGO_TEST_ASSERT_EQ(ctx, t.frame_count, (uint64_t)1);
        ERGO_TEST_ASSERT_TRUE(ctx, t.total_time > 0.0f);
    });

    suite_time.add("Time_TimeScale", [](TestContext& ctx) {
        Time t;
        t.reset();
        t.time_scale = 0.5f;
        t.tick(1.0f / 60.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, t.delta_time, (1.0f / 60.0f) * 0.5f, 0.0001f);
        ERGO_TEST_ASSERT_NEAR(ctx, t.unscaled_delta_time, 1.0f / 60.0f, 0.0001f);
    });

    suite_time.add("Time_TimeScale_Paused", [](TestContext& ctx) {
        Time t;
        t.reset();
        t.time_scale = 0.0f;
        t.tick(1.0f / 60.0f);
        ERGO_TEST_ASSERT_NEAR(ctx, t.delta_time, 0.0f, 0.0001f);
        ERGO_TEST_ASSERT_NEAR(ctx, t.unscaled_delta_time, 1.0f / 60.0f, 0.0001f);
    });

    suite_time.add("Time_FrameCount", [](TestContext& ctx) {
        Time t;
        t.reset();
        for (int i = 0; i < 10; ++i) {
            t.tick(1.0f / 60.0f);
        }
        ERGO_TEST_ASSERT_EQ(ctx, t.frame_count, (uint64_t)10);
    });

    suite_time.add("Time_TotalTime", [](TestContext& ctx) {
        Time t;
        t.reset();
        t.time_scale = 1.0f;
        for (int i = 0; i < 60; ++i) {
            t.tick(1.0f / 60.0f);
        }
        ERGO_TEST_ASSERT_NEAR(ctx, t.total_time, 1.0f, 0.01f);
    });
}

// ============================================================
// Registration
// ============================================================

void register_gameplay_tests(TestRunner& runner) {
    register_camera_tests();
    register_easing_tests();
    register_sprite_animation_tests();
    register_input_map_tests();
    register_time_tests();

    runner.add_suite(suite_camera);
    runner.add_suite(suite_easing);
    runner.add_suite(suite_sprite_animation);
    runner.add_suite(suite_input_map);
    runner.add_suite(suite_time);
}
