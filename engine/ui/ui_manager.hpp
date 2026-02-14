#pragma once
#include "ui_element.hpp"
#include <vector>
#include <memory>

struct RenderContext;

class UIManager {
public:
    void add_root(std::unique_ptr<UIElement> elem);
    void remove_root(uint64_t id);
    void clear();

    void update(float dt, Vec2f mouse_pos, bool mouse_down, bool mouse_clicked);
    void draw(RenderContext& ctx);

    UIElement* hit_test(Vec2f pos);
    UIElement* focused() const { return focused_; }

private:
    std::vector<std::unique_ptr<UIElement>> roots_;
    UIElement* focused_ = nullptr;
    UIElement* hovered_ = nullptr;

    UIElement* hit_test_recursive(UIElement* elem, Vec2f pos);
    void update_recursive(UIElement* elem, float dt);
    void draw_recursive(UIElement* elem, RenderContext& ctx);
};
