#include "ui_manager.hpp"
#include <algorithm>

void UIManager::add_root(std::unique_ptr<UIElement> elem) {
    roots_.push_back(std::move(elem));
}

void UIManager::remove_root(uint64_t id) {
    roots_.erase(
        std::remove_if(roots_.begin(), roots_.end(),
            [id](const auto& e) { return e->id == id; }),
        roots_.end());
}

void UIManager::clear() {
    roots_.clear();
    focused_ = nullptr;
    hovered_ = nullptr;
}

void UIManager::update(float dt, Vec2f mouse_pos, bool mouse_down, bool mouse_clicked) {
    UIElement* new_hovered = hit_test(mouse_pos);

    // Hover enter/exit
    if (new_hovered != hovered_) {
        if (hovered_ && hovered_->on_hover_exit) {
            hovered_->on_hover_exit();
        }
        if (new_hovered && new_hovered->on_hover_enter) {
            new_hovered->on_hover_enter();
        }
        hovered_ = new_hovered;
    }

    // Click
    if (mouse_clicked && new_hovered && new_hovered->interactive) {
        focused_ = new_hovered;
        if (new_hovered->on_click) {
            new_hovered->on_click();
        }
    }

    (void)mouse_down;

    // Update all elements
    for (auto& root : roots_) {
        update_recursive(root.get(), dt);
    }
}

void UIManager::draw(RenderContext& ctx) {
    for (auto& root : roots_) {
        draw_recursive(root.get(), ctx);
    }
}

UIElement* UIManager::hit_test(Vec2f pos) {
    // Iterate in reverse (front-to-back rendering order)
    for (auto it = roots_.rbegin(); it != roots_.rend(); ++it) {
        auto* result = hit_test_recursive(it->get(), pos);
        if (result) return result;
    }
    return nullptr;
}

UIElement* UIManager::hit_test_recursive(UIElement* elem, Vec2f pos) {
    if (!elem || !elem->visible) return nullptr;

    // Check children first (front-to-back)
    for (auto it = elem->children.rbegin(); it != elem->children.rend(); ++it) {
        auto* result = hit_test_recursive(*it, pos);
        if (result) return result;
    }

    if (elem->interactive && elem->contains(pos)) {
        return elem;
    }
    return nullptr;
}

void UIManager::update_recursive(UIElement* elem, float dt) {
    if (!elem || !elem->visible) return;
    elem->update(dt);
    for (auto* child : elem->children) {
        update_recursive(child, dt);
    }
}

void UIManager::draw_recursive(UIElement* elem, RenderContext& ctx) {
    if (!elem || !elem->visible) return;
    elem->draw(ctx);
    for (auto* child : elem->children) {
        draw_recursive(child, ctx);
    }
}
