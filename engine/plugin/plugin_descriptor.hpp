#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>

// Forward declarations
struct RenderContext;
class BehaviourHolder;

// ============================================================
// PropertyType: types that the editor can display and edit
// ============================================================

enum class PropertyType : uint8_t {
    Float,
    Int,
    Bool,
    Vec2,
    Vec3,
    Color,
    String,
    Enum,
    Asset,   // Resource reference (texture, font, sound, etc.)
};

// ============================================================
// EnumEntry: one option in an enum property
// ============================================================

struct EnumEntry {
    const char* label;   // Display label
    int32_t     value;   // Numeric value
};

// ============================================================
// PropertyDescriptor: describes a single editable property
//
// The getter/setter function pointers work with the raw void*
// returned by IBehaviour::raw_ptr(). The editor casts the
// component pointer and reads/writes the property value through
// a second void* (out_value / in_value) whose concrete type
// matches PropertyType:
//
//   Float  -> float
//   Int    -> int32_t
//   Bool   -> int32_t (0 = false, non-zero = true)
//   Vec2   -> float[2]
//   Vec3   -> float[3]
//   Color  -> uint8_t[4] (r, g, b, a)
//   String -> const char* (getter writes const char*; setter reads const char*)
//   Enum   -> int32_t
//   Asset  -> const char* (asset path)
//
// ============================================================

using PropertyGetter = void(*)(const void* component, void* out_value);
using PropertySetter = void(*)(void* component, const void* in_value);

struct PropertyDescriptor {
    const char*    name;           // Internal name (for serialization keys)
    const char*    display_name;   // Shown in the editor inspector
    PropertyType   type;

    PropertyGetter get;
    PropertySetter set;

    // Numeric range (valid when has_range == true)
    float range_min  = 0.0f;
    float range_max  = 1.0f;
    float range_step = 0.01f;
    bool  has_range  = false;

    // Enum options (valid when type == PropertyType::Enum)
    const EnumEntry* enum_entries     = nullptr;
    uint32_t         enum_entry_count = 0;

    // Tooltip shown in the editor
    const char* tooltip = nullptr;
};

// ============================================================
// GizmoDrawFunc: optional editor gizmo drawing
//
// Called by the editor's scene view to draw handles, outlines,
// or other visual helpers. Receives the raw component pointer
// and a RenderContext for immediate-mode drawing.
// ============================================================

using GizmoDrawFunc = void(*)(const void* component, RenderContext& ctx);

// ============================================================
// PluginCreateFunc: factory that adds a default instance
//
// Adds a new behaviour to the holder and returns the raw
// pointer (same as IBehaviour::raw_ptr() of the new entry).
// ============================================================

using PluginCreateFunc = void*(*)(BehaviourHolder& holder);

// ============================================================
// PluginDescriptor: THE unified plugin format
//
// Every component in the system (core and user-defined) is
// described by a PluginDescriptor. It pairs the runtime
// behaviour definition with editor metadata so that both the
// runtime and the editor can work from a single source of truth.
//
// ┌───────────────────────────────────────────────────┐
// │  PluginDescriptor                                 │
// │  ┌────────────────────┐  ┌──────────────────────┐ │
// │  │ Runtime identity   │  │ Editor metadata      │ │
// │  │  type_name         │  │  display_name        │ │
// │  │  create_default()  │  │  category            │ │
// │  │                    │  │  description         │ │
// │  │                    │  │  properties[]        │ │
// │  │                    │  │  draw_gizmo()        │ │
// │  └────────────────────┘  └──────────────────────┘ │
// └───────────────────────────────────────────────────┘
//
// ============================================================

struct PluginDescriptor {
    // ---------------------------------------------------------
    // Identity (must be unique across all registered plugins)
    // ---------------------------------------------------------
    const char* type_name;           // Matches BehaviourLike::type_name()

    // ---------------------------------------------------------
    // Editor display
    // ---------------------------------------------------------
    const char* display_name;        // Localisable name for the UI
    const char* category;            // Grouping: "Core", "Physics", "Rendering", ...
    const char* description;         // Short help text / tooltip

    // ---------------------------------------------------------
    // Property reflection
    // ---------------------------------------------------------
    const PropertyDescriptor* properties;
    uint32_t                  property_count;

    // ---------------------------------------------------------
    // Behavioural flags
    // ---------------------------------------------------------
    bool allow_multiple;             // Multiple instances on one object?
    bool removable;                  // Can the user remove this component?
    bool visible_in_add_menu;        // Show in "Add Component" menu?

    // ---------------------------------------------------------
    // Factory
    // ---------------------------------------------------------
    PluginCreateFunc create_default;  // Adds a default instance

    // ---------------------------------------------------------
    // Gizmo (optional, nullptr when not used)
    // ---------------------------------------------------------
    GizmoDrawFunc draw_gizmo;
};

// ============================================================
// Convenience: property descriptor builder helpers
//
// These are captureless lambdas that decay to function pointers,
// enabling concise property descriptors without macros.
//
// Usage:
//
//   static constexpr PropertyDescriptor props[] = {
//       ergo::prop_float<MyComp, &MyComp::speed>(
//           "speed", "Speed", 0.0f, 100.0f),
//   };
//
// ============================================================

namespace ergo {

template<typename Component, auto MemberPtr>
constexpr PropertyDescriptor prop_float(
    const char* name,
    const char* display_name,
    float range_min = 0.0f,
    float range_max = 0.0f,
    const char* tooltip = nullptr)
{
    PropertyDescriptor d{};
    d.name         = name;
    d.display_name = display_name;
    d.type         = PropertyType::Float;
    d.get = [](const void* c, void* out) {
        *static_cast<float*>(out) =
            static_cast<const Component*>(c)->*MemberPtr;
    };
    d.set = [](void* c, const void* in) {
        static_cast<Component*>(c)->*MemberPtr =
            *static_cast<const float*>(in);
    };
    d.range_min = range_min;
    d.range_max = range_max;
    d.has_range = (range_min != range_max);
    d.tooltip   = tooltip;
    return d;
}

template<typename Component, auto MemberPtr>
constexpr PropertyDescriptor prop_int(
    const char* name,
    const char* display_name,
    float range_min = 0.0f,
    float range_max = 0.0f,
    const char* tooltip = nullptr)
{
    PropertyDescriptor d{};
    d.name         = name;
    d.display_name = display_name;
    d.type         = PropertyType::Int;
    d.get = [](const void* c, void* out) {
        *static_cast<int32_t*>(out) =
            static_cast<const Component*>(c)->*MemberPtr;
    };
    d.set = [](void* c, const void* in) {
        static_cast<Component*>(c)->*MemberPtr =
            *static_cast<const int32_t*>(in);
    };
    d.range_min = range_min;
    d.range_max = range_max;
    d.has_range = (range_min != range_max);
    d.tooltip   = tooltip;
    return d;
}

template<typename Component, auto MemberPtr>
constexpr PropertyDescriptor prop_bool(
    const char* name,
    const char* display_name,
    const char* tooltip = nullptr)
{
    PropertyDescriptor d{};
    d.name         = name;
    d.display_name = display_name;
    d.type         = PropertyType::Bool;
    d.get = [](const void* c, void* out) {
        *static_cast<int32_t*>(out) =
            static_cast<const Component*>(c)->*MemberPtr ? 1 : 0;
    };
    d.set = [](void* c, const void* in) {
        static_cast<Component*>(c)->*MemberPtr =
            (*static_cast<const int32_t*>(in) != 0);
    };
    d.tooltip = tooltip;
    return d;
}

// Helper for enum properties with explicit getter/setter
template<typename Component>
constexpr PropertyDescriptor prop_enum(
    const char* name,
    const char* display_name,
    PropertyGetter getter,
    PropertySetter setter,
    const EnumEntry* entries,
    uint32_t entry_count,
    const char* tooltip = nullptr)
{
    PropertyDescriptor d{};
    d.name             = name;
    d.display_name     = display_name;
    d.type             = PropertyType::Enum;
    d.get              = getter;
    d.set              = setter;
    d.enum_entries     = entries;
    d.enum_entry_count = entry_count;
    d.tooltip          = tooltip;
    return d;
}

// Helper for Vec2 property via explicit getter/setter
inline PropertyDescriptor prop_vec2(
    const char* name,
    const char* display_name,
    PropertyGetter getter,
    PropertySetter setter,
    const char* tooltip = nullptr)
{
    PropertyDescriptor d{};
    d.name         = name;
    d.display_name = display_name;
    d.type         = PropertyType::Vec2;
    d.get          = getter;
    d.set          = setter;
    d.tooltip      = tooltip;
    return d;
}

// Helper for Vec3 property via explicit getter/setter
inline PropertyDescriptor prop_vec3(
    const char* name,
    const char* display_name,
    PropertyGetter getter,
    PropertySetter setter,
    const char* tooltip = nullptr)
{
    PropertyDescriptor d{};
    d.name         = name;
    d.display_name = display_name;
    d.type         = PropertyType::Vec3;
    d.get          = getter;
    d.set          = setter;
    d.tooltip      = tooltip;
    return d;
}

// Helper for Color property via explicit getter/setter
inline PropertyDescriptor prop_color(
    const char* name,
    const char* display_name,
    PropertyGetter getter,
    PropertySetter setter,
    const char* tooltip = nullptr)
{
    PropertyDescriptor d{};
    d.name         = name;
    d.display_name = display_name;
    d.type         = PropertyType::Color;
    d.get          = getter;
    d.set          = setter;
    d.tooltip      = tooltip;
    return d;
}

} // namespace ergo
