#include "test_framework.hpp"
#include "engine/core/sprite_animation.hpp"

TEST_CASE(SpriteAnimation_FromGrid) {
    auto anim = SpriteAnimation::from_grid(TextureHandle{1}, 4, 2, 8, 0.1f);
    ASSERT_EQ(anim.frames.size(), (size_t)8);
    ASSERT_NEAR(anim.frames[0].uv.x, 0.0f, 0.001f);
    ASSERT_NEAR(anim.frames[0].uv.w, 0.25f, 0.001f);
    ASSERT_NEAR(anim.frames[0].uv.h, 0.5f, 0.001f);
}

TEST_CASE(SpriteAnimation_FrameAdvance) {
    SpriteAnimation anim;
    anim.loop = true;
    anim.frames = {
        {Rect{0, 0, 1, 1}, 0.1f},
        {Rect{0, 0, 1, 1}, 0.1f},
        {Rect{0, 0, 1, 1}, 0.1f},
    };

    ASSERT_EQ(anim.current_frame, (uint32_t)0);
    anim.update(0.15f);
    ASSERT_EQ(anim.current_frame, (uint32_t)1);
}

TEST_CASE(SpriteAnimation_Loop) {
    SpriteAnimation anim;
    anim.loop = true;
    anim.frames = {
        {Rect{0, 0, 1, 1}, 0.1f},
        {Rect{0, 0, 1, 1}, 0.1f},
    };

    anim.update(0.25f);  // Past all frames
    ASSERT_FALSE(anim.finished);
    ASSERT_EQ(anim.current_frame, (uint32_t)0);
}

TEST_CASE(SpriteAnimation_NoLoop) {
    SpriteAnimation anim;
    anim.loop = false;
    anim.frames = {
        {Rect{0, 0, 1, 1}, 0.1f},
        {Rect{0, 0, 1, 1}, 0.1f},
    };

    anim.update(0.25f);
    ASSERT_TRUE(anim.finished);
    ASSERT_EQ(anim.current_frame, (uint32_t)1);
}

TEST_CASE(SpriteAnimation_Reset) {
    SpriteAnimation anim;
    anim.loop = false;
    anim.frames = {
        {Rect{0, 0, 1, 1}, 0.1f},
        {Rect{0, 0, 1, 1}, 0.1f},
    };

    anim.update(0.25f);
    ASSERT_TRUE(anim.finished);

    anim.reset();
    ASSERT_FALSE(anim.finished);
    ASSERT_EQ(anim.current_frame, (uint32_t)0);
    ASSERT_NEAR(anim.timer, 0.0f, 0.001f);
}

TEST_CASE(AnimationController_Play) {
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
    ASSERT_TRUE(ctrl.current_name == "idle");
    ASSERT_TRUE(ctrl.current() != nullptr);

    ctrl.play("run");
    ASSERT_TRUE(ctrl.current_name == "run");

    // Playing same animation again should not reset
    ctrl.update(0.05f);
    ctrl.play("run");
    ASSERT_TRUE(ctrl.current() != nullptr);
}
