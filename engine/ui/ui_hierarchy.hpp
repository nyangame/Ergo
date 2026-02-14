#pragma once
#include "ui_canvas.hpp"
#include "ui_image_node.hpp"
#include <functional>
#include <vector>

// ---------------------------------------------------------------------------
// UIHierarchy
//
// Manages a collection of UICanvas roots.  Provides convenience methods
// for tree operations, serialisation-ready traversal, and editor queries.
// ---------------------------------------------------------------------------
class UIHierarchy {
public:
    // Canvas management
    UICanvas* add_canvas(std::unique_ptr<UICanvas> canvas);
    void remove_canvas(uint64_t id);
    UICanvas* canvas_at(int index) const;
    int canvas_count() const { return static_cast<int>(canvases_.size()); }

    // Global find (searches all canvases)
    UINode* find_by_id(uint64_t id) const;
    UINode* find_by_name(const std::string& name) const;

    // Tree manipulation
    static UINode* reparent(UINode* node, UINode* new_parent);

    // Depth-first traversal (pre-order) over every node in every canvas.
    // Callback: void(UINode* node, int depth)
    using TraversalCallback = std::function<void(UINode*, int)>;
    void traverse(const TraversalCallback& cb) const;

    // Update & draw all canvases
    void update_all(float dt);
    void draw_all(RenderContext& ctx);

    // Flat list of all nodes (useful for editor hierarchy panel).
    // Returns {node_ptr, depth} pairs in tree-order.
    struct FlatEntry {
        UINode* node;
        int depth;
    };
    std::vector<FlatEntry> flatten() const;

    void clear();

private:
    std::vector<std::unique_ptr<UICanvas>> canvases_;

    static void traverse_recursive(UINode* node, int depth,
                                   const TraversalCallback& cb);
};

// Global UI hierarchy instance
inline UIHierarchy g_ui_hierarchy;
