#include "ui_image_node.hpp"
#include "system/renderer/vulkan/vk_renderer.hpp"

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------
UIImageNode::UIImageNode(std::string name)
    : UINode(std::move(name)) {}

UIImageNode::~UIImageNode() = default;

// ---------------------------------------------------------------------------
// Native size
// ---------------------------------------------------------------------------
void UIImageNode::set_native_size(uint32_t w, uint32_t h) {
    native_w_ = w;
    native_h_ = h;
}

void UIImageNode::set_size_to_native() {
    if (native_w_ > 0 && native_h_ > 0) {
        rect_.size_delta = {static_cast<float>(native_w_),
                            static_cast<float>(native_h_)};
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void UIImageNode::draw(RenderContext& ctx, const WorldRect& parent_rect) {
    if (!is_active() || !is_visible()) return;

    WorldRect wr = compute_world_rect(parent_rect);

    if (texture_.valid()) {
        if (preserve_aspect_ && native_w_ > 0 && native_h_ > 0) {
            // Fit image within bounds while keeping aspect ratio
            float aspect = static_cast<float>(native_w_) /
                           static_cast<float>(native_h_);
            float rect_aspect = wr.w / wr.h;

            float draw_w, draw_h;
            if (aspect > rect_aspect) {
                // Width-limited
                draw_w = wr.w;
                draw_h = wr.w / aspect;
            } else {
                // Height-limited
                draw_h = wr.h;
                draw_w = wr.h * aspect;
            }

            float offset_x = (wr.w - draw_w) * 0.5f;
            float offset_y = (wr.h - draw_h) * 0.5f;

            ctx.draw_sprite({wr.x + offset_x, wr.y + offset_y},
                            {draw_w, draw_h}, texture_, uv_);
        } else {
            ctx.draw_sprite({wr.x, wr.y}, {wr.w, wr.h}, texture_, uv_);
        }
    }

    // Draw children
    for (auto& child : children_) {
        child->draw(ctx, wr);
    }
}
