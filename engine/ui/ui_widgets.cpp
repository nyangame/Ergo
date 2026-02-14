#include "ui_widgets.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"

void UILabel::draw(RenderContext& ctx) {
    Vec2f cp = computed_position();
    ctx.draw_text(cp, text.c_str(), color, font_scale);
}

void UIButton::draw(RenderContext& ctx) {
    Vec2f cp = computed_position();
    Color bg = is_pressed ? pressed_color : (is_hovered ? hover_color : normal_color);
    ctx.draw_rect(cp, size, bg, true);
    // Center text (approximate)
    Vec2f text_pos = {cp.x + size.w * 0.1f, cp.y + size.h * 0.25f};
    ctx.draw_text(text_pos, text.c_str(), text_color, 1.0f);
}

void UIImage::draw(RenderContext& ctx) {
    Vec2f cp = computed_position();
    ctx.draw_sprite(cp, size, texture, uv);
}

void UISlider::draw(RenderContext& ctx) {
    Vec2f cp = computed_position();
    // Track
    ctx.draw_rect(cp, size, track_color, true);
    // Handle
    float ratio = (value - min_value) / (max_value - min_value);
    float handle_w = size.w * 0.05f;
    float handle_x = cp.x + ratio * (size.w - handle_w);
    ctx.draw_rect({handle_x, cp.y}, {handle_w, size.h}, handle_color, true);
}

void UIProgressBar::draw(RenderContext& ctx) {
    Vec2f cp = computed_position();
    ctx.draw_rect(cp, size, bg_color, true);
    float fill_w = size.w * progress;
    if (fill_w > 0.0f) {
        ctx.draw_rect(cp, {fill_w, size.h}, fill_color, true);
    }
}

void UIPanel::draw(RenderContext& ctx) {
    Vec2f cp = computed_position();
    ctx.draw_rect(cp, size, bg_color, true);
}

void UITextInput::draw(RenderContext& ctx) {
    Vec2f cp = computed_position();
    ctx.draw_rect(cp, size, bg_color, true);
    Vec2f text_pos = {cp.x + 4.0f, cp.y + size.h * 0.25f};
    if (text.empty()) {
        ctx.draw_text(text_pos, placeholder.c_str(), placeholder_color, 1.0f);
    } else {
        ctx.draw_text(text_pos, text.c_str(), text_color, 1.0f);
    }
}
