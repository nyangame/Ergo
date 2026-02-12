#pragma once

struct RenderContext;
struct Player;

// In-game state
struct InGameScene {
    int score = 0;
    int enemy_spawn_timer = 0;

    void enter();
    void update(float dt);
    void draw(RenderContext& ctx);
    void exit();
};
