#include "editor_api.h"
#include "system/renderer/vulkan/vk_renderer.hpp"
#include "engine/math/transform3d.hpp"
#include "engine/math/mat4.hpp"
#include "engine/core/game_object.hpp"

#include <vector>
#include <unordered_map>
#include <string>
#include <cstring>
#include <cmath>
#include <mutex>

// ============================================================
// Internal editor state
// ============================================================

namespace {

struct EditorCamera {
    Vec3f eye    {0.0f, 5.0f, -10.0f};
    Vec3f target {0.0f, 0.0f, 0.0f};
    Vec3f up     {0.0f, 1.0f, 0.0f};
    float fov_degrees  = 60.0f;
    float near_plane   = 0.1f;
    float far_plane    = 1000.0f;
};

struct RenderTarget {
    uint64_t        id             = 0;
    void*           native_handle  = nullptr;
    uint32_t        width          = 0;
    uint32_t        height         = 0;
    ErgoRenderMode  mode           = ERGO_RENDER_MODE_SCENE;
    EditorCamera    camera;
    // Vulkan surface/swapchain would be stored here in a full implementation
};

struct EditorState {
    bool initialized = false;
    uint64_t next_rt_id = 1;
    std::unordered_map<uint64_t, RenderTarget> render_targets;

    // Scene objects managed by the editor
    uint64_t next_object_id = 1;
    std::unordered_map<uint64_t, GameObject> objects;

    // Temporary buffers for string returns
    std::string temp_name;

    std::mutex mutex;
};

EditorState g_editor;

Vec3f to_vec3(const ErgoVec3& v) { return {v.x, v.y, v.z}; }
ErgoVec3 from_vec3(const Vec3f& v) { return {v.x, v.y, v.z}; }

} // anonymous namespace

// ============================================================
// Engine lifecycle
// ============================================================

extern "C" {

int ergo_editor_init(void) {
    std::lock_guard lock(g_editor.mutex);
    if (g_editor.initialized) return 0;

    // TODO: Initialize VulkanRenderer for editor use
    //       Create Vulkan instance & device once, shared across render targets.
    g_editor.initialized = true;
    return 1;
}

void ergo_editor_shutdown(void) {
    std::lock_guard lock(g_editor.mutex);
    if (!g_editor.initialized) return;

    g_editor.render_targets.clear();
    g_editor.objects.clear();
    g_editor.initialized = false;
}

void ergo_editor_tick(float dt) {
    std::lock_guard lock(g_editor.mutex);
    // Advance engine simulation by dt seconds
    // TODO: step physics, update tasks, etc.
    (void)dt;
}

// ============================================================
// Render target management
// ============================================================

ErgoRenderTargetHandle ergo_editor_create_render_target(
    void* native_window_handle, uint32_t width, uint32_t height,
    ErgoRenderMode mode)
{
    std::lock_guard lock(g_editor.mutex);

    uint64_t id = g_editor.next_rt_id++;
    RenderTarget rt;
    rt.id            = id;
    rt.native_handle = native_window_handle;
    rt.width         = width;
    rt.height        = height;
    rt.mode          = mode;

    // TODO: Create VkSurfaceKHR from native_window_handle
    //       Create swapchain, framebuffers, etc.

    g_editor.render_targets[id] = rt;
    return {id};
}

void ergo_editor_destroy_render_target(ErgoRenderTargetHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    // TODO: Destroy Vulkan surface/swapchain
    g_editor.render_targets.erase(handle.id);
}

void ergo_editor_resize_render_target(
    ErgoRenderTargetHandle handle, uint32_t width, uint32_t height)
{
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.render_targets.find(handle.id);
    if (it == g_editor.render_targets.end()) return;
    it->second.width  = width;
    it->second.height = height;
    // TODO: Recreate swapchain with new extent
}

void ergo_editor_render_frame(ErgoRenderTargetHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.render_targets.find(handle.id);
    if (it == g_editor.render_targets.end()) return;

    auto& rt = it->second;

    // TODO: Build view/projection matrices from camera
    // TODO: Acquire swapchain image
    // TODO: Record render commands (draw meshes, grid, gizmos based on mode)
    // TODO: Submit and present

    (void)rt;
}

// ============================================================
// Camera
// ============================================================

void ergo_editor_set_camera(
    ErgoRenderTargetHandle handle,
    ErgoVec3 eye, ErgoVec3 target, ErgoVec3 up,
    float fov_degrees, float near_plane, float far_plane)
{
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.render_targets.find(handle.id);
    if (it == g_editor.render_targets.end()) return;

    auto& cam        = it->second.camera;
    cam.eye          = to_vec3(eye);
    cam.target       = to_vec3(target);
    cam.up           = to_vec3(up);
    cam.fov_degrees  = fov_degrees;
    cam.near_plane   = near_plane;
    cam.far_plane    = far_plane;
}

// ============================================================
// Scene query
// ============================================================

uint32_t ergo_editor_get_object_count(void) {
    std::lock_guard lock(g_editor.mutex);
    return static_cast<uint32_t>(g_editor.objects.size());
}

uint32_t ergo_editor_get_objects(
    ErgoGameObjectHandle* out_handles, uint32_t max_count)
{
    std::lock_guard lock(g_editor.mutex);
    uint32_t count = 0;
    for (auto& [id, _] : g_editor.objects) {
        if (count >= max_count) break;
        out_handles[count++] = {id};
    }
    return count;
}

const char* ergo_editor_get_object_name(ErgoGameObjectHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.objects.find(handle.id);
    if (it == g_editor.objects.end()) return "";
    g_editor.temp_name = it->second.name_;
    return g_editor.temp_name.c_str();
}

ErgoTransform3D ergo_editor_get_object_transform(ErgoGameObjectHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.objects.find(handle.id);
    if (it == g_editor.objects.end()) {
        return {{0,0,0}, {0,0,0,1}, {1,1,1}};
    }
    // For now, convert 2D transform to 3D representation
    auto& t = it->second.transform_;
    ErgoTransform3D result;
    result.position = {t.position.x, t.position.y, 0.0f};
    result.rotation = {0.0f, 0.0f, std::sin(t.rotation * 0.5f),
                       std::cos(t.rotation * 0.5f)};
    result.scale    = {t.size.w, t.size.h, 1.0f};
    return result;
}

void ergo_editor_set_object_transform(
    ErgoGameObjectHandle handle, ErgoTransform3D transform)
{
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.objects.find(handle.id);
    if (it == g_editor.objects.end()) return;

    auto& t = it->second.transform_;
    t.position = {transform.position.x, transform.position.y};
    t.rotation = std::atan2(transform.rotation.z, transform.rotation.w) * 2.0f;
    t.size     = {transform.scale.x, transform.scale.y};
}

// ============================================================
// Component query  (stub implementation)
// ============================================================

uint32_t ergo_editor_get_component_count(ErgoGameObjectHandle object) {
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.objects.find(object.id);
    if (it == g_editor.objects.end()) return 0;
    return static_cast<uint32_t>(it->second.components_.size());
}

uint32_t ergo_editor_get_components(
    ErgoGameObjectHandle object,
    ErgoComponentInfo* out_infos, uint32_t max_count)
{
    std::lock_guard lock(g_editor.mutex);
    auto it = g_editor.objects.find(object.id);
    if (it == g_editor.objects.end()) return 0;

    uint32_t count = 0;
    for (auto& [type_idx, _] : it->second.components_) {
        if (count >= max_count) break;
        out_infos[count].name           = type_idx.name();
        out_infos[count].type_name      = type_idx.name();
        out_infos[count].property_count = 0; // TODO: reflection
        count++;
    }
    return count;
}

uint32_t ergo_editor_get_component_properties(
    ErgoGameObjectHandle object, const char* component_name,
    ErgoPropertyInfo* out_props, uint32_t max_count)
{
    // TODO: Implement property reflection system
    (void)object;
    (void)component_name;
    (void)out_props;
    (void)max_count;
    return 0;
}

int ergo_editor_set_component_property(
    ErgoGameObjectHandle object, const char* component_name,
    const char* property_name, const ErgoPropertyInfo* value)
{
    // TODO: Implement property setting via reflection
    (void)object;
    (void)component_name;
    (void)property_name;
    (void)value;
    return 0;
}

// ============================================================
// Object picking
// ============================================================

ErgoGameObjectHandle ergo_editor_pick_object(
    ErgoRenderTargetHandle render_target,
    float screen_x, float screen_y)
{
    std::lock_guard lock(g_editor.mutex);
    // TODO: Cast ray from camera through (screen_x, screen_y),
    //       test against all object bounding volumes.
    (void)render_target;
    (void)screen_x;
    (void)screen_y;
    return {0};
}

} // extern "C"
