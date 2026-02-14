#include "ui_node.hpp"
#include <atomic>
#include <cassert>

// ---------------------------------------------------------------------------
// ID generation
// ---------------------------------------------------------------------------
static std::atomic<uint64_t> s_next_id{1};

uint64_t UINode::next_id() {
    return s_next_id.fetch_add(1, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------
UINode::UINode(std::string name)
    : id_(next_id()), name_(std::move(name)) {}

UINode::~UINode() = default;

// ---------------------------------------------------------------------------
// Hierarchy
// ---------------------------------------------------------------------------
UINode* UINode::add_child(std::unique_ptr<UINode> child) {
    if (!child) return nullptr;

    // Detach from previous parent
    if (child->parent_) {
        child->parent_->remove_child(child.get());
    }

    child->parent_ = this;
    auto* ptr = child.get();
    children_.push_back(std::move(child));
    return ptr;
}

std::unique_ptr<UINode> UINode::remove_child(UINode* child) {
    if (!child) return nullptr;

    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if (it->get() == child) {
            auto owned = std::move(*it);
            children_.erase(it);
            owned->parent_ = nullptr;
            return owned;
        }
    }
    return nullptr;
}

void UINode::set_sibling_index(int index) {
    if (!parent_) return;

    auto& siblings = parent_->children_;
    int current = sibling_index();
    if (current < 0 || current == index) return;

    int count = static_cast<int>(siblings.size());
    index = std::clamp(index, 0, count - 1);

    auto owned = std::move(siblings[current]);
    siblings.erase(siblings.begin() + current);
    siblings.insert(siblings.begin() + index, std::move(owned));
}

int UINode::sibling_index() const {
    if (!parent_) return -1;

    const auto& siblings = parent_->children_;
    for (int i = 0; i < static_cast<int>(siblings.size()); ++i) {
        if (siblings[i].get() == this) return i;
    }
    return -1;
}

UINode* UINode::child_at(int index) const {
    if (index < 0 || index >= static_cast<int>(children_.size()))
        return nullptr;
    return children_[index].get();
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------
UINode* UINode::find_by_name(const std::string& target) const {
    for (const auto& child : children_) {
        if (child->name_ == target) return child.get();
        if (auto* found = child->find_by_name(target)) return found;
    }
    return nullptr;
}

UINode* UINode::find_by_id(uint64_t target) const {
    for (const auto& child : children_) {
        if (child->id_ == target) return child.get();
        if (auto* found = child->find_by_id(target)) return found;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// World rect computation
//
// Follows the Unity RectTransform model:
//   If anchor_min == anchor_max (point anchor):
//     size = size_delta
//     center = parent_rect_anchor_pos + position - pivot * size
//   If anchor_min != anchor_max (stretch):
//     edges defined by anchor * parent_size + offset
//     size_delta acts as inset (negative = grow, positive = shrink)
// ---------------------------------------------------------------------------
WorldRect UINode::compute_world_rect(const WorldRect& pr) const {
    const auto& rt = rect_;

    float anchor_left   = pr.x + rt.anchor_min.x * pr.w;
    float anchor_top    = pr.y + rt.anchor_min.y * pr.h;
    float anchor_right  = pr.x + rt.anchor_max.x * pr.w;
    float anchor_bottom = pr.y + rt.anchor_max.y * pr.h;

    float anchor_w = anchor_right - anchor_left;
    float anchor_h = anchor_bottom - anchor_top;

    WorldRect wr;

    if (anchor_w < 0.001f && anchor_h < 0.001f) {
        // Point anchor: fixed size
        wr.w = rt.size_delta.w;
        wr.h = rt.size_delta.h;
        wr.x = anchor_left + rt.position.x - rt.pivot.x * wr.w;
        wr.y = anchor_top  + rt.position.y - rt.pivot.y * wr.h;
    } else if (anchor_w < 0.001f) {
        // Stretch vertical, fixed horizontal
        wr.w = rt.size_delta.w;
        wr.h = anchor_h - rt.size_delta.h;
        wr.x = anchor_left + rt.position.x - rt.pivot.x * wr.w;
        wr.y = anchor_top + rt.size_delta.h * 0.5f + rt.position.y;
    } else if (anchor_h < 0.001f) {
        // Stretch horizontal, fixed vertical
        wr.w = anchor_w - rt.size_delta.w;
        wr.h = rt.size_delta.h;
        wr.x = anchor_left + rt.size_delta.w * 0.5f + rt.position.x;
        wr.y = anchor_top + rt.position.y - rt.pivot.y * wr.h;
    } else {
        // Stretch both axes
        wr.x = anchor_left - rt.size_delta.w * 0.5f + rt.position.x;
        wr.y = anchor_top  - rt.size_delta.h * 0.5f + rt.position.y;
        wr.w = anchor_w + rt.size_delta.w;
        wr.h = anchor_h + rt.size_delta.h;
    }

    return wr;
}

// ---------------------------------------------------------------------------
// Update / draw
// ---------------------------------------------------------------------------
void UINode::update(float dt) {
    if (!active_) return;
    for (auto& child : children_) {
        child->update(dt);
    }
}

void UINode::draw(RenderContext& ctx, const WorldRect& parent_rect) {
    if (!active_ || !visible_) return;

    WorldRect wr = compute_world_rect(parent_rect);
    for (auto& child : children_) {
        child->draw(ctx, wr);
    }
}

// ---------------------------------------------------------------------------
// Hit test
// ---------------------------------------------------------------------------
UINode* UINode::hit_test(Vec2f pos, const WorldRect& parent_rect) {
    if (!active_ || !visible_) return nullptr;

    WorldRect wr = compute_world_rect(parent_rect);

    // Test children in reverse order (front-most first)
    for (int i = static_cast<int>(children_.size()) - 1; i >= 0; --i) {
        if (auto* hit = children_[i]->hit_test(pos, wr)) {
            return hit;
        }
    }

    // Test self
    if (wr.contains(pos)) return this;
    return nullptr;
}
