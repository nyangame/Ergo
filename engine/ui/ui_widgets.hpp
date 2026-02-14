#pragma once
#include "ui_element.hpp"
#include "../resource/texture_handle.hpp"
#include <string>

// Forward declaration
struct RenderContext;

struct UILabel : UIElement {
    std::string text;
    Color color{255, 255, 255, 255};
    float font_scale = 1.0f;

    void draw(RenderContext& ctx) override;
};

struct UIButton : UIElement {
    std::string text;
    Color normal_color{100, 100, 100, 255};
    Color hover_color{140, 140, 140, 255};
    Color pressed_color{80, 80, 80, 255};
    Color text_color{255, 255, 255, 255};

    bool is_hovered = false;
    bool is_pressed = false;

    void draw(RenderContext& ctx) override;
};

struct UIImage : UIElement {
    TextureHandle texture;
    Rect uv{0, 0, 1, 1};
    Color tint{255, 255, 255, 255};

    void draw(RenderContext& ctx) override;
};

struct UISlider : UIElement {
    float min_value = 0.0f;
    float max_value = 1.0f;
    float value = 0.5f;
    Color track_color{60, 60, 60, 255};
    Color handle_color{200, 200, 200, 255};

    std::function<void(float)> on_value_changed;

    void draw(RenderContext& ctx) override;
};

struct UIProgressBar : UIElement {
    float progress = 0.0f;  // 0.0 ~ 1.0
    Color bg_color{40, 40, 40, 255};
    Color fill_color{0, 180, 0, 255};

    void draw(RenderContext& ctx) override;
};

struct UIPanel : UIElement {
    Color bg_color{30, 30, 30, 200};

    void draw(RenderContext& ctx) override;
};

struct UITextInput : UIElement {
    std::string text;
    std::string placeholder;
    Color bg_color{50, 50, 50, 255};
    Color text_color{255, 255, 255, 255};
    Color placeholder_color{128, 128, 128, 255};
    bool focused = false;
    size_t cursor_pos = 0;

    std::function<void(const std::string&)> on_submit;

    void draw(RenderContext& ctx) override;
};
