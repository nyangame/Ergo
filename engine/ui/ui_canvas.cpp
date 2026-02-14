#include "ui_canvas.hpp"
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------
UICanvas::UICanvas(std::string name)
    : UINode(std::move(name)) {}

UICanvas::~UICanvas() = default;

// ---------------------------------------------------------------------------
// Screen size & scale factor
// ---------------------------------------------------------------------------
void UICanvas::set_screen_size(float w, float h) {
    screen_size_ = {w, h};
    recalculate_scale();
}

void UICanvas::recalculate_scale() {
    if (scale_mode_ == CanvasScaleMode::DotByDot) {
        scale_factor_ = 1.0f;
        return;
    }

    // ScaleWithScreen
    float ref_w = reference_resolution_.w;
    float ref_h = reference_resolution_.h;
    if (ref_w <= 0.0f || ref_h <= 0.0f) {
        scale_factor_ = 1.0f;
        return;
    }

    float scale_w = screen_size_.w / ref_w;
    float scale_h = screen_size_.h / ref_h;

    switch (screen_match_mode_) {
        case ScreenMatchMode::MatchWidth:
            scale_factor_ = scale_w;
            break;
        case ScreenMatchMode::MatchHeight:
            scale_factor_ = scale_h;
            break;
        case ScreenMatchMode::MatchMinAxis:
            scale_factor_ = std::min(scale_w, scale_h);
            break;
        case ScreenMatchMode::MatchMaxAxis:
            scale_factor_ = std::max(scale_w, scale_h);
            break;
    }

    if (scale_factor_ <= 0.0f) scale_factor_ = 1.0f;
}

Size2f UICanvas::canvas_size() const {
    if (scale_factor_ <= 0.0f) return screen_size_;
    return {screen_size_.w / scale_factor_, screen_size_.h / scale_factor_};
}

WorldRect UICanvas::root_rect() const {
    Size2f cs = canvas_size();
    return {0.0f, 0.0f, cs.w, cs.h};
}

// ---------------------------------------------------------------------------
// Update & draw
// ---------------------------------------------------------------------------
void UICanvas::update_all(float dt) {
    if (!is_active()) return;
    for (auto& child : children_) {
        child->update(dt);
    }
}

void UICanvas::draw_all(RenderContext& ctx) {
    if (!is_active() || !is_visible()) return;

    WorldRect rr = root_rect();
    for (auto& child : children_) {
        child->draw(ctx, rr);
    }
}

// ---------------------------------------------------------------------------
// Hit test
// ---------------------------------------------------------------------------
UINode* UICanvas::hit_test_screen(Vec2f screen_pos) {
    if (!is_active() || !is_visible()) return nullptr;

    // Convert screen coords to canvas coords
    Vec2f canvas_pos = {screen_pos.x / scale_factor_,
                        screen_pos.y / scale_factor_};

    WorldRect rr = root_rect();

    // Test children in reverse order
    for (int i = static_cast<int>(children_.size()) - 1; i >= 0; --i) {
        if (auto* hit = children_[i]->hit_test(canvas_pos, rr)) {
            return hit;
        }
    }
    return nullptr;
}
