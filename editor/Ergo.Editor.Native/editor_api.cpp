#include "editor_api.h"
#include "editor_renderer.hpp"
#include "engine/math/transform3d.hpp"
#include "engine/math/mat4.hpp"
#include "engine/core/game_object.hpp"

#include <unordered_map>
#include <string>
#include <cmath>
#include <mutex>

// ============================================================
// Internal editor state
// ============================================================

namespace {

struct EditorState {
    EditorRenderer renderer;

    // Scene objects managed by the editor
    uint64_t next_object_id = 1;
    std::unordered_map<uint64_t, GameObject> objects;

    // Temporary buffer for string returns (kept alive until next call)
    std::string temp_name;

    std::mutex mutex;
};

EditorState g_editor;

} // anonymous namespace

// ============================================================
// Engine lifecycle
// ============================================================

extern "C" {

int ergo_editor_init(void) {
    std::lock_guard lock(g_editor.mutex);
    if (g_editor.renderer.is_initialized()) return 1;
    return g_editor.renderer.initialize() ? 1 : 0;
}

void ergo_editor_shutdown(void) {
    std::lock_guard lock(g_editor.mutex);
    g_editor.renderer.shutdown();
    g_editor.objects.clear();
}

void ergo_editor_tick(float dt) {
    std::lock_guard lock(g_editor.mutex);
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

    auto rm = (mode == ERGO_RENDER_MODE_SCENE)
                  ? RenderMode::Scene
                  : RenderMode::Game;

    uint64_t id = g_editor.renderer.create_render_target(
        native_window_handle, width, height, rm);

    return {id};
}

void ergo_editor_destroy_render_target(ErgoRenderTargetHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    g_editor.renderer.destroy_render_target(handle.id);
}

void ergo_editor_resize_render_target(
    ErgoRenderTargetHandle handle, uint32_t width, uint32_t height)
{
    std::lock_guard lock(g_editor.mutex);
    g_editor.renderer.resize_render_target(handle.id, width, height);
}

void ergo_editor_render_frame(ErgoRenderTargetHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    g_editor.renderer.render_frame(handle.id);
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
    EditorCamera cam;
    cam.eye[0] = eye.x;    cam.eye[1] = eye.y;    cam.eye[2] = eye.z;
    cam.target[0] = target.x; cam.target[1] = target.y; cam.target[2] = target.z;
    cam.up[0] = up.x;      cam.up[1] = up.y;      cam.up[2] = up.z;
    cam.fov_degrees = fov_degrees;
    cam.near_plane  = near_plane;
    cam.far_plane   = far_plane;
    g_editor.renderer.set_camera(handle.id, cam);
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
// Component query
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
        out_infos[count].property_count = 0;
        count++;
    }
    return count;
}

uint32_t ergo_editor_get_component_properties(
    ErgoGameObjectHandle object, const char* component_name,
    ErgoPropertyInfo* out_props, uint32_t max_count)
{
    // TODO: Implement property reflection system
    (void)object; (void)component_name; (void)out_props; (void)max_count;
    return 0;
}

int ergo_editor_set_component_property(
    ErgoGameObjectHandle object, const char* component_name,
    const char* property_name, const ErgoPropertyInfo* value)
{
    // TODO: Implement property setting via reflection
    (void)object; (void)component_name; (void)property_name; (void)value;
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
    // TODO: Ray cast from camera through (screen_x, screen_y)
    (void)render_target; (void)screen_x; (void)screen_y;
    return {0};
}

} // extern "C"
