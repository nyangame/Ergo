#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #define ERGO_EDITOR_API __declspec(dllexport)
#else
    #define ERGO_EDITOR_API __attribute__((visibility("default")))
#endif

// ============================================================
// Plugin property types (mirrors PropertyType enum in C++)
// ============================================================

typedef enum {
    ERGO_PLUGIN_PROP_FLOAT  = 0,
    ERGO_PLUGIN_PROP_INT    = 1,
    ERGO_PLUGIN_PROP_BOOL   = 2,
    ERGO_PLUGIN_PROP_VEC2   = 3,
    ERGO_PLUGIN_PROP_VEC3   = 4,
    ERGO_PLUGIN_PROP_COLOR  = 5,
    ERGO_PLUGIN_PROP_STRING = 6,
    ERGO_PLUGIN_PROP_ENUM   = 7,
    ERGO_PLUGIN_PROP_ASSET  = 8,
} ErgoPluginPropertyType;

// ============================================================
// Plugin info (flat C struct for P/Invoke)
// ============================================================

typedef struct {
    const char* type_name;
    const char* display_name;
    const char* category;
    const char* description;
    uint32_t    property_count;
    int32_t     allow_multiple;
    int32_t     removable;
    int32_t     visible_in_add_menu;
} ErgoPluginInfo;

// ============================================================
// Plugin property info (flat C struct)
// ============================================================

typedef struct {
    const char*             name;
    const char*             display_name;
    ErgoPluginPropertyType  type;
    float                   range_min;
    float                   range_max;
    float                   range_step;
    int32_t                 has_range;
    const char*             tooltip;
    uint32_t                enum_entry_count;
} ErgoPluginPropertyInfo;

// ============================================================
// Enum entry info
// ============================================================

typedef struct {
    const char* label;
    int32_t     value;
} ErgoPluginEnumEntry;

// ============================================================
// Plugin registry queries
// ============================================================

/// Get the total number of registered plugins.
ERGO_EDITOR_API uint32_t ergo_plugin_get_count(void);

/// Get info for all registered plugins.
/// Returns the number actually written (up to max_count).
ERGO_EDITOR_API uint32_t ergo_plugin_get_all(
    ErgoPluginInfo* out_infos, uint32_t max_count);

/// Get info for a single plugin by type_name.
/// Returns 1 if found, 0 otherwise.
ERGO_EDITOR_API int32_t ergo_plugin_get_info(
    const char* type_name, ErgoPluginInfo* out_info);

/// Get plugins filtered by category.
/// Returns the number actually written.
ERGO_EDITOR_API uint32_t ergo_plugin_get_by_category(
    const char* category,
    ErgoPluginInfo* out_infos, uint32_t max_count);

// ============================================================
// Plugin property queries
// ============================================================

/// Get property descriptors for a plugin type.
/// Returns the number actually written (up to max_count).
ERGO_EDITOR_API uint32_t ergo_plugin_get_properties(
    const char* type_name,
    ErgoPluginPropertyInfo* out_props, uint32_t max_count);

/// Get enum entries for a specific enum property.
/// Returns the number actually written (up to max_count).
ERGO_EDITOR_API uint32_t ergo_plugin_get_enum_entries(
    const char* type_name,
    const char* property_name,
    ErgoPluginEnumEntry* out_entries, uint32_t max_count);

// ============================================================
// Plugin property read/write on live behaviour instances
//
// These operate on a behaviour attached to a game object.
// The object handle identifies the game object, and type_name
// identifies which behaviour on that object to access.
// ============================================================

typedef struct { uint64_t id; } ErgoGameObjectHandle;

/// Read a float property value from a behaviour instance.
/// Returns 1 on success, 0 if not found.
ERGO_EDITOR_API int32_t ergo_plugin_get_float(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    float* out_value);

/// Write a float property value to a behaviour instance.
ERGO_EDITOR_API int32_t ergo_plugin_set_float(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    float value);

/// Read an int property value from a behaviour instance.
ERGO_EDITOR_API int32_t ergo_plugin_get_int(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    int32_t* out_value);

/// Write an int property value to a behaviour instance.
ERGO_EDITOR_API int32_t ergo_plugin_set_int(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    int32_t value);

// ============================================================
// Plugin instance management
// ============================================================

/// Add a default instance of a plugin to a game object.
/// Returns 1 on success, 0 on failure.
ERGO_EDITOR_API int32_t ergo_plugin_add_to_object(
    ErgoGameObjectHandle object,
    const char* type_name);

/// Remove a plugin instance from a game object.
/// Returns 1 on success, 0 on failure.
ERGO_EDITOR_API int32_t ergo_plugin_remove_from_object(
    ErgoGameObjectHandle object,
    const char* type_name);

/// Check if a game object has a specific plugin.
/// Returns 1 if present, 0 otherwise.
ERGO_EDITOR_API int32_t ergo_plugin_object_has(
    ErgoGameObjectHandle object,
    const char* type_name);

#ifdef __cplusplus
}
#endif
