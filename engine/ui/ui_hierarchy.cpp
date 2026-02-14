#include "ui_hierarchy.hpp"

// ---------------------------------------------------------------------------
// Canvas management
// ---------------------------------------------------------------------------
UICanvas* UIHierarchy::add_canvas(std::unique_ptr<UICanvas> canvas) {
    if (!canvas) return nullptr;
    auto* ptr = canvas.get();
    canvases_.push_back(std::move(canvas));
    return ptr;
}

void UIHierarchy::remove_canvas(uint64_t id) {
    canvases_.erase(
        std::remove_if(canvases_.begin(), canvases_.end(),
                        [id](const auto& c) { return c->id() == id; }),
        canvases_.end());
}

UICanvas* UIHierarchy::canvas_at(int index) const {
    if (index < 0 || index >= static_cast<int>(canvases_.size()))
        return nullptr;
    return canvases_[index].get();
}

// ---------------------------------------------------------------------------
// Global search
// ---------------------------------------------------------------------------
UINode* UIHierarchy::find_by_id(uint64_t id) const {
    for (const auto& canvas : canvases_) {
        if (canvas->id() == id) return canvas.get();
        if (auto* found = canvas->find_by_id(id)) return found;
    }
    return nullptr;
}

UINode* UIHierarchy::find_by_name(const std::string& name) const {
    for (const auto& canvas : canvases_) {
        if (canvas->name() == name) return canvas.get();
        if (auto* found = canvas->find_by_name(name)) return found;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Reparent
// ---------------------------------------------------------------------------
UINode* UIHierarchy::reparent(UINode* node, UINode* new_parent) {
    if (!node || !new_parent) return nullptr;
    if (node->parent() == new_parent) return node;

    UINode* old_parent = node->parent();
    if (!old_parent) return nullptr; // Cannot reparent a root canvas

    auto owned = old_parent->remove_child(node);
    if (!owned) return nullptr;

    return new_parent->add_child(std::move(owned));
}

// ---------------------------------------------------------------------------
// Traversal
// ---------------------------------------------------------------------------
void UIHierarchy::traverse(const TraversalCallback& cb) const {
    for (const auto& canvas : canvases_) {
        cb(canvas.get(), 0);
        traverse_recursive(canvas.get(), 0, cb);
    }
}

void UIHierarchy::traverse_recursive(UINode* node, int depth,
                                      const TraversalCallback& cb) {
    for (const auto& child : node->children()) {
        cb(child.get(), depth + 1);
        traverse_recursive(child.get(), depth + 1, cb);
    }
}

// ---------------------------------------------------------------------------
// Flatten
// ---------------------------------------------------------------------------
std::vector<UIHierarchy::FlatEntry> UIHierarchy::flatten() const {
    std::vector<FlatEntry> result;
    traverse([&result](UINode* node, int depth) {
        result.push_back({node, depth});
    });
    return result;
}

// ---------------------------------------------------------------------------
// Update & draw
// ---------------------------------------------------------------------------
void UIHierarchy::update_all(float dt) {
    for (auto& canvas : canvases_) {
        canvas->update_all(dt);
    }
}

void UIHierarchy::draw_all(RenderContext& ctx) {
    for (auto& canvas : canvases_) {
        canvas->draw_all(ctx);
    }
}

void UIHierarchy::clear() {
    canvases_.clear();
}
