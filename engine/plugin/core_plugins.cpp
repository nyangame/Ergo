#include "core_plugins.hpp"
#include "plugin_registry.hpp"
#include "../core/behaviour/behaviour.hpp"

namespace ergo {

// ============================================================
// Transform2DPlugin
// ============================================================

static const PropertyDescriptor s_transform2d_props[] = {
    prop_float<Transform2DPlugin, &Transform2DPlugin::pos_x>(
        "pos_x", "Position X", 0.0f, 0.0f, "X position in world space"),
    prop_float<Transform2DPlugin, &Transform2DPlugin::pos_y>(
        "pos_y", "Position Y", 0.0f, 0.0f, "Y position in world space"),
    prop_float<Transform2DPlugin, &Transform2DPlugin::rotation>(
        "rotation", "Rotation", -6.28318f, 6.28318f, "Rotation in radians"),
    prop_float<Transform2DPlugin, &Transform2DPlugin::size_w>(
        "size_w", "Width", 0.001f, 10000.0f, "Width scale"),
    prop_float<Transform2DPlugin, &Transform2DPlugin::size_h>(
        "size_h", "Height", 0.001f, 10000.0f, "Height scale"),
};

static const PluginDescriptor s_transform2d_descriptor = {
    .type_name          = "Transform2D",
    .display_name       = "Transform 2D",
    .category           = "Core",
    .description        = "2D position, rotation, and scale of the game object.",
    .properties         = s_transform2d_props,
    .property_count     = sizeof(s_transform2d_props) / sizeof(s_transform2d_props[0]),
    .allow_multiple     = false,
    .removable          = false,
    .visible_in_add_menu = false,
    .create_default     = [](BehaviourHolder& holder) -> void* {
        return &holder.add<Transform2DPlugin>();
    },
    .draw_gizmo         = nullptr,
};

const PluginDescriptor& Transform2DPlugin::plugin_descriptor() {
    return s_transform2d_descriptor;
}

// ============================================================
// CircleColliderPlugin
// ============================================================

static const PropertyDescriptor s_circle_collider_props[] = {
    prop_float<CircleColliderPlugin, &CircleColliderPlugin::radius>(
        "radius", "Radius", 0.001f, 10000.0f, "Collision circle radius"),
    prop_float<CircleColliderPlugin, &CircleColliderPlugin::offset_x>(
        "offset_x", "Offset X", 0.0f, 0.0f, "X offset from transform center"),
    prop_float<CircleColliderPlugin, &CircleColliderPlugin::offset_y>(
        "offset_y", "Offset Y", 0.0f, 0.0f, "Y offset from transform center"),
    prop_int<CircleColliderPlugin, &CircleColliderPlugin::tag>(
        "tag", "Tag", 0.0f, 16.0f, "Collider tag for filtering"),
    prop_bool<CircleColliderPlugin, &CircleColliderPlugin::enabled>(
        "enabled", "Enabled", "Enable/disable collision detection"),
};

static const PluginDescriptor s_circle_collider_descriptor = {
    .type_name          = "CircleCollider",
    .display_name       = "Circle Collider",
    .category           = "Physics",
    .description        = "Circle-shaped collision area for 2D physics.",
    .properties         = s_circle_collider_props,
    .property_count     = sizeof(s_circle_collider_props) / sizeof(s_circle_collider_props[0]),
    .allow_multiple     = true,
    .removable          = true,
    .visible_in_add_menu = true,
    .create_default     = [](BehaviourHolder& holder) -> void* {
        return &holder.add<CircleColliderPlugin>();
    },
    .draw_gizmo         = nullptr,
};

const PluginDescriptor& CircleColliderPlugin::plugin_descriptor() {
    return s_circle_collider_descriptor;
}

// ============================================================
// AABBColliderPlugin
// ============================================================

static const PropertyDescriptor s_aabb_collider_props[] = {
    prop_float<AABBColliderPlugin, &AABBColliderPlugin::half_w>(
        "half_w", "Half Width", 0.001f, 10000.0f, "Half-extent in X"),
    prop_float<AABBColliderPlugin, &AABBColliderPlugin::half_h>(
        "half_h", "Half Height", 0.001f, 10000.0f, "Half-extent in Y"),
    prop_float<AABBColliderPlugin, &AABBColliderPlugin::offset_x>(
        "offset_x", "Offset X", 0.0f, 0.0f, "X offset from transform center"),
    prop_float<AABBColliderPlugin, &AABBColliderPlugin::offset_y>(
        "offset_y", "Offset Y", 0.0f, 0.0f, "Y offset from transform center"),
    prop_int<AABBColliderPlugin, &AABBColliderPlugin::tag>(
        "tag", "Tag", 0.0f, 16.0f, "Collider tag for filtering"),
    prop_bool<AABBColliderPlugin, &AABBColliderPlugin::enabled>(
        "enabled", "Enabled", "Enable/disable collision detection"),
};

static const PluginDescriptor s_aabb_collider_descriptor = {
    .type_name          = "AABBCollider",
    .display_name       = "AABB Collider",
    .category           = "Physics",
    .description        = "Axis-aligned bounding box collision area for 2D physics.",
    .properties         = s_aabb_collider_props,
    .property_count     = sizeof(s_aabb_collider_props) / sizeof(s_aabb_collider_props[0]),
    .allow_multiple     = true,
    .removable          = true,
    .visible_in_add_menu = true,
    .create_default     = [](BehaviourHolder& holder) -> void* {
        return &holder.add<AABBColliderPlugin>();
    },
    .draw_gizmo         = nullptr,
};

const PluginDescriptor& AABBColliderPlugin::plugin_descriptor() {
    return s_aabb_collider_descriptor;
}

// ============================================================
// SpriteRendererPlugin
// ============================================================

static const PropertyDescriptor s_sprite_renderer_props[] = {
    prop_float<SpriteRendererPlugin, &SpriteRendererPlugin::color_r>(
        "color_r", "Color R", 0.0f, 255.0f, "Red channel"),
    prop_float<SpriteRendererPlugin, &SpriteRendererPlugin::color_g>(
        "color_g", "Color G", 0.0f, 255.0f, "Green channel"),
    prop_float<SpriteRendererPlugin, &SpriteRendererPlugin::color_b>(
        "color_b", "Color B", 0.0f, 255.0f, "Blue channel"),
    prop_float<SpriteRendererPlugin, &SpriteRendererPlugin::color_a>(
        "color_a", "Color A", 0.0f, 255.0f, "Alpha channel"),
    prop_bool<SpriteRendererPlugin, &SpriteRendererPlugin::flip_x>(
        "flip_x", "Flip X", "Mirror the sprite horizontally"),
    prop_bool<SpriteRendererPlugin, &SpriteRendererPlugin::flip_y>(
        "flip_y", "Flip Y", "Mirror the sprite vertically"),
    prop_int<SpriteRendererPlugin, &SpriteRendererPlugin::sort_order>(
        "sort_order", "Sort Order", -1000.0f, 1000.0f,
        "Draw order within the same layer"),
};

static const PluginDescriptor s_sprite_renderer_descriptor = {
    .type_name          = "SpriteRenderer",
    .display_name       = "Sprite Renderer",
    .category           = "Rendering",
    .description        = "Renders a 2D sprite from a texture asset.",
    .properties         = s_sprite_renderer_props,
    .property_count     = sizeof(s_sprite_renderer_props) / sizeof(s_sprite_renderer_props[0]),
    .allow_multiple     = false,
    .removable          = true,
    .visible_in_add_menu = true,
    .create_default     = [](BehaviourHolder& holder) -> void* {
        return &holder.add<SpriteRendererPlugin>();
    },
    .draw_gizmo         = nullptr,
};

const PluginDescriptor& SpriteRendererPlugin::plugin_descriptor() {
    return s_sprite_renderer_descriptor;
}

// ============================================================
// CameraPlugin
// ============================================================

static const PropertyDescriptor s_camera_props[] = {
    prop_float<CameraPlugin, &CameraPlugin::fov>(
        "fov", "Field of View", 1.0f, 179.0f, "Vertical field of view in degrees"),
    prop_float<CameraPlugin, &CameraPlugin::near_plane>(
        "near_plane", "Near Plane", 0.001f, 1000.0f, "Near clipping distance"),
    prop_float<CameraPlugin, &CameraPlugin::far_plane>(
        "far_plane", "Far Plane", 1.0f, 100000.0f, "Far clipping distance"),
    prop_float<CameraPlugin, &CameraPlugin::viewport_x>(
        "viewport_x", "Viewport X", 0.0f, 1.0f, "Viewport left edge (normalized)"),
    prop_float<CameraPlugin, &CameraPlugin::viewport_y>(
        "viewport_y", "Viewport Y", 0.0f, 1.0f, "Viewport top edge (normalized)"),
    prop_float<CameraPlugin, &CameraPlugin::viewport_w>(
        "viewport_w", "Viewport W", 0.0f, 1.0f, "Viewport width (normalized)"),
    prop_float<CameraPlugin, &CameraPlugin::viewport_h>(
        "viewport_h", "Viewport H", 0.0f, 1.0f, "Viewport height (normalized)"),
    prop_int<CameraPlugin, &CameraPlugin::priority>(
        "priority", "Priority", -100.0f, 100.0f, "Render priority (lower = first)"),
    prop_bool<CameraPlugin, &CameraPlugin::orthographic>(
        "orthographic", "Orthographic", "Use orthographic projection"),
    prop_float<CameraPlugin, &CameraPlugin::ortho_size>(
        "ortho_size", "Ortho Size", 0.1f, 1000.0f,
        "Half-height of the orthographic view volume"),
};

static const PluginDescriptor s_camera_descriptor = {
    .type_name          = "Camera",
    .display_name       = "Camera",
    .category           = "Core",
    .description        = "Camera component that defines a viewport for rendering.",
    .properties         = s_camera_props,
    .property_count     = sizeof(s_camera_props) / sizeof(s_camera_props[0]),
    .allow_multiple     = true,
    .removable          = true,
    .visible_in_add_menu = true,
    .create_default     = [](BehaviourHolder& holder) -> void* {
        return &holder.add<CameraPlugin>();
    },
    .draw_gizmo         = nullptr,
};

const PluginDescriptor& CameraPlugin::plugin_descriptor() {
    return s_camera_descriptor;
}

// ============================================================
// register_core_plugins
// ============================================================

void register_core_plugins(PluginRegistry& registry) {
    registry.register_plugin(&s_transform2d_descriptor);
    registry.register_plugin(&s_circle_collider_descriptor);
    registry.register_plugin(&s_aabb_collider_descriptor);
    registry.register_plugin(&s_sprite_renderer_descriptor);
    registry.register_plugin(&s_camera_descriptor);
}

} // namespace ergo
