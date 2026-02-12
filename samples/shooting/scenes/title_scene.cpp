#include "title_scene.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"

void TitleScene::enter() {
    start_requested = false;
}

void TitleScene::update(float /*dt*/) {
    // Transition to game when start is requested
    // Input checking is handled through the engine API
}

void TitleScene::draw(RenderContext& ctx) {
    // Draw title text
    ctx.draw_text({250.0f, 200.0f}, "SHOOTING GAME", Color{255, 255, 255, 255}, 2.0f);
    ctx.draw_text({280.0f, 350.0f}, "PRESS SPACE TO START", Color{200, 200, 200, 255}, 1.0f);
}

void TitleScene::exit() {
}
