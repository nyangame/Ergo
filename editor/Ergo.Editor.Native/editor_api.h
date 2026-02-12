#pragma once
#include "game_interface/engine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #define ERGO_EDITOR_API __declspec(dllexport)
#else
    #define ERGO_EDITOR_API __attribute__((visibility("default")))
#endif

// ---------------------------------------------------------------------------
// Handle types
// ---------------------------------------------------------------------------
typedef struct { uint64_t id; } ErgoRenderTargetHandle;
typedef struct { uint64_t id; } ErgoGameObjectHandle;

// ---------------------------------------------------------------------------
// 3D types (C-compatible mirrors of engine math types)
// ---------------------------------------------------------------------------
typedef struct { float x, y, z; }    ErgoVec3;
typedef struct { float x, y, z, w; } ErgoQuat;

typedef struct {
    ErgoVec3 position;
    ErgoQuat rotation;
    ErgoVec3 scale;
} ErgoTransform3D;

// ---------------------------------------------------------------------------
// Component descriptor (passed to managed side for property display)
// ---------------------------------------------------------------------------
typedef struct {
    const char* name;
    const char* type_name;
    uint32_t    property_count;
} ErgoComponentInfo;

typedef enum {
    ERGO_PROP_FLOAT,
    ERGO_PROP_INT,
    ERGO_PROP_BOOL,
    ERGO_PROP_VEC3,
    ERGO_PROP_STRING,
    ERGO_PROP_COLOR,
} ErgoPropertyType;

typedef struct {
    const char*      name;
    ErgoPropertyType type;
    union {
        float       f;
        int32_t     i;
        int32_t     b;
        ErgoVec3    v3;
        ErgoColor   color;
        const char* str;
    } value;
} ErgoPropertyInfo;

// ---------------------------------------------------------------------------
// Render target mode (determines which overlays are drawn)
// ---------------------------------------------------------------------------
typedef enum {
    ERGO_RENDER_MODE_SCENE,  // Grid, gizmos, selection outlines
    ERGO_RENDER_MODE_GAME,   // Clean game view, no editor overlays
} ErgoRenderMode;

// ---------------------------------------------------------------------------
// Engine lifecycle
// ---------------------------------------------------------------------------
ERGO_EDITOR_API int  ergo_editor_init(void);
ERGO_EDITOR_API void ergo_editor_shutdown(void);
ERGO_EDITOR_API void ergo_editor_tick(float dt);

// ---------------------------------------------------------------------------
// Render target management
// Each render target corresponds to one native window/view surface that
// Vulkan renders into. Scene view and game view each get their own target.
// ---------------------------------------------------------------------------
ERGO_EDITOR_API ErgoRenderTargetHandle ergo_editor_create_render_target(
    void* native_window_handle, uint32_t width, uint32_t height,
    ErgoRenderMode mode);

ERGO_EDITOR_API void ergo_editor_destroy_render_target(
    ErgoRenderTargetHandle handle);

ERGO_EDITOR_API void ergo_editor_resize_render_target(
    ErgoRenderTargetHandle handle, uint32_t width, uint32_t height);

ERGO_EDITOR_API void ergo_editor_render_frame(
    ErgoRenderTargetHandle handle);

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------
ERGO_EDITOR_API void ergo_editor_set_camera(
    ErgoRenderTargetHandle handle,
    ErgoVec3 eye, ErgoVec3 target, ErgoVec3 up,
    float fov_degrees, float near_plane, float far_plane);

// ---------------------------------------------------------------------------
// Scene query
// ---------------------------------------------------------------------------
ERGO_EDITOR_API uint32_t ergo_editor_get_object_count(void);

ERGO_EDITOR_API uint32_t ergo_editor_get_objects(
    ErgoGameObjectHandle* out_handles, uint32_t max_count);

ERGO_EDITOR_API const char* ergo_editor_get_object_name(
    ErgoGameObjectHandle handle);

ERGO_EDITOR_API ErgoTransform3D ergo_editor_get_object_transform(
    ErgoGameObjectHandle handle);

ERGO_EDITOR_API void ergo_editor_set_object_transform(
    ErgoGameObjectHandle handle, ErgoTransform3D transform);

// ---------------------------------------------------------------------------
// Component query
// ---------------------------------------------------------------------------
ERGO_EDITOR_API uint32_t ergo_editor_get_component_count(
    ErgoGameObjectHandle object);

ERGO_EDITOR_API uint32_t ergo_editor_get_components(
    ErgoGameObjectHandle object,
    ErgoComponentInfo* out_infos, uint32_t max_count);

ERGO_EDITOR_API uint32_t ergo_editor_get_component_properties(
    ErgoGameObjectHandle object, const char* component_name,
    ErgoPropertyInfo* out_props, uint32_t max_count);

ERGO_EDITOR_API int ergo_editor_set_component_property(
    ErgoGameObjectHandle object, const char* component_name,
    const char* property_name, const ErgoPropertyInfo* value);

// ---------------------------------------------------------------------------
// Object picking (ray cast from screen coordinates)
// ---------------------------------------------------------------------------
ERGO_EDITOR_API ErgoGameObjectHandle ergo_editor_pick_object(
    ErgoRenderTargetHandle render_target,
    float screen_x, float screen_y);

#ifdef __cplusplus
}
#endif
