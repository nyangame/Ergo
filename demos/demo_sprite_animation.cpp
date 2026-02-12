#include "demo_framework.hpp"
#include "engine/core/sprite_animation.hpp"
#include <cstdio>

DEMO(SpriteAnimation_Playback) {
    TextureHandle tex{42};

    SpriteAnimation anim = SpriteAnimation::from_grid(tex, 4, 2, 8, 0.1f);
    anim.loop = true;

    std::printf("  Frames: %zu, texture=%llu, loop=%s\n",
                anim.frames.size(), (unsigned long long)anim.texture.id,
                anim.loop ? "yes" : "no");

    for (int i = 0; i < 12; ++i) {
        float dt = 0.05f;
        anim.update(dt);
        const auto& uv = anim.current_uv();
        std::printf("    t=%.2f frame=%u uv=(%.2f, %.2f, %.2f, %.2f)\n",
                    dt * (i + 1), anim.current_frame,
                    uv.x, uv.y, uv.w, uv.h);
    }
}

DEMO(SpriteAnimation_NonLoop) {
    SpriteAnimation anim;
    anim.texture = TextureHandle{1};
    anim.loop = false;
    anim.frames = {
        {Rect{0, 0, 0.5f, 0.5f}, 0.2f},
        {Rect{0.5f, 0, 0.5f, 0.5f}, 0.2f},
        {Rect{0, 0.5f, 0.5f, 0.5f}, 0.2f},
    };

    std::printf("  Non-loop animation with %zu frames:\n", anim.frames.size());
    for (int i = 0; i < 10; ++i) {
        anim.update(0.1f);
        std::printf("    Step %d: frame=%u finished=%s\n",
                    i, anim.current_frame, anim.finished ? "yes" : "no");
    }
}

DEMO(AnimationController_Switch) {
    AnimationController ctrl;

    SpriteAnimation idle;
    idle.texture = TextureHandle{1};
    idle.frames = {{Rect{0, 0, 1, 1}, 0.5f}, {Rect{0, 0, 1, 1}, 0.5f}};
    idle.loop = true;

    SpriteAnimation run;
    run.texture = TextureHandle{1};
    run.frames = {{Rect{0, 0, 0.25f, 1}, 0.1f}, {Rect{0.25f, 0, 0.25f, 1}, 0.1f},
                  {Rect{0.5f, 0, 0.25f, 1}, 0.1f}, {Rect{0.75f, 0, 0.25f, 1}, 0.1f}};
    run.loop = true;

    ctrl.animations["idle"] = idle;
    ctrl.animations["run"] = run;

    ctrl.play("idle");
    std::printf("  Playing: '%s'\n", ctrl.current_name.c_str());
    ctrl.update(0.3f);

    ctrl.play("run");
    std::printf("  Switched to: '%s'\n", ctrl.current_name.c_str());
    ctrl.update(0.05f);

    auto* current = ctrl.current();
    if (current) {
        std::printf("  Current frame: %u\n", current->current_frame);
    }
}
