#include "engine_context.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"
#include "system/input/desktop_input.hpp"

// Static references for C callback functions
static VulkanRenderer* s_renderer = nullptr;
static DesktopInput* s_input = nullptr;

static void api_draw_rect(ErgoVec2 pos, ErgoSize2 size, ErgoColor color, int filled) {
    if (!s_renderer || !s_renderer->context()) return;
    s_renderer->context()->draw_rect(
        {pos.x, pos.y}, {size.w, size.h},
        {color.r, color.g, color.b, color.a}, filled != 0);
}

static void api_draw_circle(ErgoVec2 center, float radius, ErgoColor color, int filled) {
    if (!s_renderer || !s_renderer->context()) return;
    s_renderer->context()->draw_circle(
        {center.x, center.y}, radius,
        {color.r, color.g, color.b, color.a}, filled != 0);
}

static void api_draw_text(ErgoVec2 pos, const char* text, ErgoColor color, float scale) {
    if (!s_renderer || !s_renderer->context()) return;
    s_renderer->context()->draw_text(
        {pos.x, pos.y}, text,
        {color.r, color.g, color.b, color.a}, scale);
}

static int api_is_key_down(uint32_t key) {
    if (!s_input) return 0;
    return s_input->is_key_down(key) ? 1 : 0;
}

static int api_is_key_pressed(uint32_t key) {
    if (!s_input) return 0;
    return s_input->is_key_pressed(key) ? 1 : 0;
}

static ErgoVec2 api_mouse_position() {
    if (!s_input) return {0.0f, 0.0f};
    Vec2f pos = s_input->mouse_position();
    return {pos.x, pos.y};
}

static ErgoTextureHandle api_load_texture(const char* path) {
    if (!s_renderer) return {0};
    auto handle = s_renderer->load_texture(path);
    return {handle.id};
}

static void api_unload_texture(ErgoTextureHandle handle) {
    if (!s_renderer) return;
    s_renderer->unload_texture({handle.id});
}

ErgoEngineAPI build_engine_api(VulkanRenderer& renderer, DesktopInput& input) {
    s_renderer = &renderer;
    s_input = &input;

    ErgoEngineAPI api{};
    api.draw_rect = api_draw_rect;
    api.draw_circle = api_draw_circle;
    api.draw_text = api_draw_text;
    api.is_key_down = api_is_key_down;
    api.is_key_pressed = api_is_key_pressed;
    api.mouse_position = api_mouse_position;
    api.load_texture = api_load_texture;
    api.unload_texture = api_unload_texture;
    return api;
}
