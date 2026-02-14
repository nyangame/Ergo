#pragma once
#include "../math/vec2.hpp"
#include "../math/size2.hpp"
#include "../math/color.hpp"
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

struct RenderContext;

// ---------------------------------------------------------------------------
// UIRectTransform
//
// Unity uGUI-inspired rect transform. Defines how a node is positioned
// and sized relative to its parent's rect.
//
//   anchor_min / anchor_max : normalised (0..1) positions within the parent.
//     When min == max the node uses a fixed size (size_delta).
//     When min != max the node stretches and size_delta acts as insets.
//   pivot      : normalised origin within the node itself (0..1).
//   position   : offset from the anchored position (pixels).
//   size_delta : fixed size or inset depending on anchor spread.
// ---------------------------------------------------------------------------
struct UIRectTransform {
    Vec2f anchor_min{0.5f, 0.5f};
    Vec2f anchor_max{0.5f, 0.5f};
    Vec2f pivot{0.5f, 0.5f};
    Vec2f position{0.0f, 0.0f};
    Size2f size_delta{100.0f, 100.0f};
};

// ---------------------------------------------------------------------------
// WorldRect
//
// Fully resolved screen-space rectangle for a node.
// ---------------------------------------------------------------------------
struct WorldRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    bool contains(Vec2f point) const {
        return point.x >= x && point.x <= x + w &&
               point.y >= y && point.y <= y + h;
    }
};

// ---------------------------------------------------------------------------
// UINode
//
// Base class for all UI editor nodes.  Mirrors Unity's GameObject with
// a RectTransform component.  Owns its children (unique_ptr).
// ---------------------------------------------------------------------------
class UINode {
public:
    explicit UINode(std::string name = "Node");
    virtual ~UINode();

    // Identification
    uint64_t id() const { return id_; }
    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    // Active / visible
    bool is_active() const { return active_; }
    void set_active(bool v) { active_ = v; }
    bool is_visible() const { return visible_; }
    void set_visible(bool v) { visible_ = v; }

    // Transform
    UIRectTransform& rect_transform() { return rect_; }
    const UIRectTransform& rect_transform() const { return rect_; }

    // Hierarchy
    UINode* parent() const { return parent_; }
    const std::vector<std::unique_ptr<UINode>>& children() const { return children_; }

    UINode* add_child(std::unique_ptr<UINode> child);
    std::unique_ptr<UINode> remove_child(UINode* child);
    void set_sibling_index(int index);
    int sibling_index() const;
    int child_count() const { return static_cast<int>(children_.size()); }
    UINode* child_at(int index) const;

    // Search
    UINode* find_by_name(const std::string& name) const;
    UINode* find_by_id(uint64_t id) const;

    // Computed world rect (call after canvas resolves scale)
    WorldRect compute_world_rect(const WorldRect& parent_rect) const;

    // Traversal
    virtual void update(float dt);
    virtual void draw(RenderContext& ctx, const WorldRect& parent_rect);

    // Hit test (deepest visible interactive child first)
    UINode* hit_test(Vec2f pos, const WorldRect& parent_rect);

protected:
    uint64_t id_;
    std::string name_;
    bool active_ = true;
    bool visible_ = true;
    UIRectTransform rect_;

    UINode* parent_ = nullptr;
    std::vector<std::unique_ptr<UINode>> children_;

    static uint64_t next_id();
};
