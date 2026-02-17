#pragma once

#include "plugin_descriptor.hpp"
#include "../core/concepts.hpp"
#include "../math/vec2.hpp"
#include "../math/size2.hpp"
#include "../math/transform.hpp"

class PluginRegistry;

// ============================================================
// Core plugin components
//
// Each core component is a BehaviourLike type that also provides
// a PluginDescriptor (satisfying PluginLike). The editor and
// runtime both work from this unified format.
//
// These serve as reference implementations of the plugin format
// and provide the fundamental building blocks for game objects.
// ============================================================

namespace ergo {

// ============================================================
// Transform2DPlugin
//
// Wraps Transform2D as a behaviour plugin. Every game object
// has a transform, so this plugin is non-removable and does not
// allow multiples.
// ============================================================

struct Transform2DPlugin {
    // --- Data ---
    float pos_x    = 0.0f;
    float pos_y    = 0.0f;
    float rotation = 0.0f;  // radians
    float size_w   = 1.0f;
    float size_h   = 1.0f;

    // --- BehaviourLike ---
    static constexpr std::string_view type_name() { return "Transform2D"; }
    void start()          {}
    void update(float)    {}
    void release()        {}

    // --- Sync helpers ---
    void write_to(Transform2D& t) const {
        t.position = {pos_x, pos_y};
        t.rotation = rotation;
        t.size     = {size_w, size_h};
    }

    void read_from(const Transform2D& t) {
        pos_x    = t.position.x;
        pos_y    = t.position.y;
        rotation = t.rotation;
        size_w   = t.size.w;
        size_h   = t.size.h;
    }

    // --- PluginLike ---
    static const PluginDescriptor& plugin_descriptor();
};

static_assert(BehaviourLike<Transform2DPlugin>);

// ============================================================
// CircleColliderPlugin
//
// A circle-shaped collider component.
// ============================================================

struct CircleColliderPlugin {
    // --- Data ---
    float    radius   = 1.0f;
    float    offset_x = 0.0f;
    float    offset_y = 0.0f;
    int32_t  tag      = 0;
    bool     enabled  = true;

    // --- BehaviourLike ---
    static constexpr std::string_view type_name() { return "CircleCollider"; }
    void start()          {}
    void update(float)    {}
    void release()        {}

    // --- PluginLike ---
    static const PluginDescriptor& plugin_descriptor();
};

static_assert(BehaviourLike<CircleColliderPlugin>);

// ============================================================
// AABBColliderPlugin
//
// An axis-aligned bounding box collider component.
// ============================================================

struct AABBColliderPlugin {
    // --- Data ---
    float    half_w   = 0.5f;
    float    half_h   = 0.5f;
    float    offset_x = 0.0f;
    float    offset_y = 0.0f;
    int32_t  tag      = 0;
    bool     enabled  = true;

    // --- BehaviourLike ---
    static constexpr std::string_view type_name() { return "AABBCollider"; }
    void start()          {}
    void update(float)    {}
    void release()        {}

    // --- PluginLike ---
    static const PluginDescriptor& plugin_descriptor();
};

static_assert(BehaviourLike<AABBColliderPlugin>);

// ============================================================
// SpriteRendererPlugin
//
// Renders a 2D sprite from a texture asset.
// ============================================================

struct SpriteRendererPlugin {
    // --- Data ---
    float    color_r   = 255;
    float    color_g   = 255;
    float    color_b   = 255;
    float    color_a   = 255;
    bool     flip_x    = false;
    bool     flip_y    = false;
    int32_t  sort_order = 0;

    // --- BehaviourLike ---
    static constexpr std::string_view type_name() { return "SpriteRenderer"; }
    void start()          {}
    void update(float)    {}
    void release()        {}

    // --- PluginLike ---
    static const PluginDescriptor& plugin_descriptor();
};

static_assert(BehaviourLike<SpriteRendererPlugin>);

// ============================================================
// CameraPlugin
//
// Defines a camera viewport for rendering.
// ============================================================

struct CameraPlugin {
    // --- Data ---
    float fov         = 60.0f;
    float near_plane  = 0.1f;
    float far_plane   = 1000.0f;
    float viewport_x  = 0.0f;
    float viewport_y  = 0.0f;
    float viewport_w  = 1.0f;
    float viewport_h  = 1.0f;
    int32_t priority  = 0;
    bool  orthographic = false;
    float ortho_size   = 5.0f;

    // --- BehaviourLike ---
    static constexpr std::string_view type_name() { return "Camera"; }
    void start()          {}
    void update(float)    {}
    void release()        {}

    // --- PluginLike ---
    static const PluginDescriptor& plugin_descriptor();
};

static_assert(BehaviourLike<CameraPlugin>);

// ============================================================
// Registration entry point
// ============================================================

/// Registers all core plugins into the given registry.
void register_core_plugins(PluginRegistry& registry);

} // namespace ergo
