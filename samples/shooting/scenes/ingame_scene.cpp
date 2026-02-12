#include "ingame_scene.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"
#include <cstdio>
#include <cstring>

void InGameScene::enter() {
    score = 0;
    enemy_spawn_timer = 0;
}

void InGameScene::update(float dt) {
    // Enemy spawn timer
    ++enemy_spawn_timer;

    // Spawn enemies periodically
    // Actual spawning is done through the task system
}

void InGameScene::draw(RenderContext& ctx) {
    // Draw score
    char score_text[64];
    std::snprintf(score_text, sizeof(score_text), "SCORE: %d", score);
    ctx.draw_text({10.0f, 10.0f}, score_text, Color{255, 255, 255, 255}, 1.0f);
}

void InGameScene::exit() {
}
