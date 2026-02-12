#pragma once

struct RenderContext;

// Title screen state
struct TitleScene {
    bool start_requested = false;

    void enter();
    void update(float dt);
    void draw(RenderContext& ctx);
    void exit();
};
