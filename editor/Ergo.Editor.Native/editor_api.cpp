#include "editor_api.h"
#include "editor_renderer.hpp"
#include "engine/math/transform3d.hpp"
#include "engine/math/mat4.hpp"
#include "engine/core/game_object.hpp"
#include "engine/ui/ui_hierarchy.hpp"
#include "engine/resource/resource_manager.hpp"
#include "engine/resource/image_loader.hpp"

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

// ============================================================
// UI Editor Hierarchy
// ============================================================

ErgoUINodeHandle ergo_ui_create_canvas(const char* name) {
    std::lock_guard lock(g_editor.mutex);
    auto canvas = std::make_unique<UICanvas>(name ? name : "Canvas");
    uint64_t id = canvas->id();
    g_ui_hierarchy.add_canvas(std::move(canvas));
    return {id};
}

void ergo_ui_remove_canvas(ErgoUINodeHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    g_ui_hierarchy.remove_canvas(handle.id);
}

uint32_t ergo_ui_get_canvas_count(void) {
    std::lock_guard lock(g_editor.mutex);
    return static_cast<uint32_t>(g_ui_hierarchy.canvas_count());
}

void ergo_ui_set_canvas_scale_mode(
    ErgoUINodeHandle canvas, ErgoUIScaleMode mode)
{
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(canvas.id);
    auto* c = dynamic_cast<UICanvas*>(node);
    if (!c) return;

    c->set_scale_mode(mode == ERGO_UI_SCALE_WITH_SCREEN
                          ? CanvasScaleMode::ScaleWithScreen
                          : CanvasScaleMode::DotByDot);
}

ErgoUIScaleMode ergo_ui_get_canvas_scale_mode(ErgoUINodeHandle canvas) {
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(canvas.id);
    auto* c = dynamic_cast<UICanvas*>(node);
    if (!c) return ERGO_UI_SCALE_DOT_BY_DOT;

    return c->scale_mode() == CanvasScaleMode::ScaleWithScreen
               ? ERGO_UI_SCALE_WITH_SCREEN
               : ERGO_UI_SCALE_DOT_BY_DOT;
}

void ergo_ui_set_canvas_reference_resolution(
    ErgoUINodeHandle canvas, float width, float height)
{
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(canvas.id);
    auto* c = dynamic_cast<UICanvas*>(node);
    if (!c) return;
    c->set_reference_resolution({width, height});
}

void ergo_ui_set_canvas_screen_match_mode(
    ErgoUINodeHandle canvas, ErgoUIScreenMatchMode mode)
{
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(canvas.id);
    auto* c = dynamic_cast<UICanvas*>(node);
    if (!c) return;

    ScreenMatchMode sm;
    switch (mode) {
        case ERGO_UI_MATCH_WIDTH:    sm = ScreenMatchMode::MatchWidth; break;
        case ERGO_UI_MATCH_HEIGHT:   sm = ScreenMatchMode::MatchHeight; break;
        case ERGO_UI_MATCH_MAX_AXIS: sm = ScreenMatchMode::MatchMaxAxis; break;
        default:                     sm = ScreenMatchMode::MatchMinAxis; break;
    }
    c->set_screen_match_mode(sm);
}

void ergo_ui_set_canvas_screen_size(
    ErgoUINodeHandle canvas, float width, float height)
{
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(canvas.id);
    auto* c = dynamic_cast<UICanvas*>(node);
    if (!c) return;
    c->set_screen_size(width, height);
}

ErgoUINodeHandle ergo_ui_create_node(
    ErgoUINodeHandle parent, const char* name)
{
    std::lock_guard lock(g_editor.mutex);
    auto* parent_node = g_ui_hierarchy.find_by_id(parent.id);
    if (!parent_node) return {0};

    auto node = std::make_unique<UINode>(name ? name : "Node");
    uint64_t id = node->id();
    parent_node->add_child(std::move(node));
    return {id};
}

ErgoUINodeHandle ergo_ui_create_image_node(
    ErgoUINodeHandle parent, const char* name, const char* texture_path)
{
    std::lock_guard lock(g_editor.mutex);
    auto* parent_node = g_ui_hierarchy.find_by_id(parent.id);
    if (!parent_node) return {0};

    auto img = std::make_unique<UIImageNode>(name ? name : "Image");

    if (texture_path && texture_path[0] != '\0') {
        TextureHandle tex = g_resources.load_texture(texture_path);
        img->set_texture(tex);

        ImageData data = load_image(texture_path);
        if (data.valid()) {
            img->set_native_size(data.width, data.height);
            img->set_size_to_native();
        }
    }

    uint64_t id = img->id();
    parent_node->add_child(std::move(img));
    return {id};
}

void ergo_ui_remove_node(ErgoUINodeHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(handle.id);
    if (!node || !node->parent()) return;
    node->parent()->remove_child(node);
}

const char* ergo_ui_get_node_name(ErgoUINodeHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(handle.id);
    if (!node) return "";
    g_editor.temp_name = node->name();
    return g_editor.temp_name.c_str();
}

void ergo_ui_set_node_name(ErgoUINodeHandle handle, const char* name) {
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(handle.id);
    if (!node || !name) return;
    node->set_name(name);
}

ErgoUIRectTransform ergo_ui_get_rect_transform(ErgoUINodeHandle handle) {
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(handle.id);
    if (!node) return {};

    const auto& rt = node->rect_transform();
    ErgoUIRectTransform result;
    result.anchor_min = {rt.anchor_min.x, rt.anchor_min.y};
    result.anchor_max = {rt.anchor_max.x, rt.anchor_max.y};
    result.pivot      = {rt.pivot.x, rt.pivot.y};
    result.position   = {rt.position.x, rt.position.y};
    result.size_delta  = {rt.size_delta.w, rt.size_delta.h};
    return result;
}

void ergo_ui_set_rect_transform(
    ErgoUINodeHandle handle, ErgoUIRectTransform rect)
{
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(handle.id);
    if (!node) return;

    auto& rt = node->rect_transform();
    rt.anchor_min = {rect.anchor_min.x, rect.anchor_min.y};
    rt.anchor_max = {rect.anchor_max.x, rect.anchor_max.y};
    rt.pivot      = {rect.pivot.x, rect.pivot.y};
    rt.position   = {rect.position.x, rect.position.y};
    rt.size_delta = {rect.size_delta.w, rect.size_delta.h};
}

void ergo_ui_set_node_active(ErgoUINodeHandle handle, int32_t active) {
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(handle.id);
    if (node) node->set_active(active != 0);
}

void ergo_ui_set_node_visible(ErgoUINodeHandle handle, int32_t visible) {
    std::lock_guard lock(g_editor.mutex);
    auto* node = g_ui_hierarchy.find_by_id(handle.id);
    if (node) node->set_visible(visible != 0);
}

uint32_t ergo_ui_get_hierarchy_count(void) {
    std::lock_guard lock(g_editor.mutex);
    return static_cast<uint32_t>(g_ui_hierarchy.flatten().size());
}

uint32_t ergo_ui_get_hierarchy(
    ErgoUINodeInfo* out_infos, uint32_t max_count)
{
    std::lock_guard lock(g_editor.mutex);
    auto flat = g_ui_hierarchy.flatten();
    uint32_t count = 0;

    for (const auto& entry : flat) {
        if (count >= max_count) break;

        auto* node = entry.node;
        auto& info = out_infos[count];
        info.handle      = {node->id()};
        info.parent       = {node->parent() ? node->parent()->id() : 0};
        info.name         = node->name().c_str();
        info.depth        = entry.depth;
        info.child_count  = node->child_count();
        info.active       = node->is_active() ? 1 : 0;
        info.visible      = node->is_visible() ? 1 : 0;

        // Determine node type
        if (dynamic_cast<UICanvas*>(node))
            info.node_type = ERGO_UI_NODE_CANVAS;
        else if (dynamic_cast<UIImageNode*>(node))
            info.node_type = ERGO_UI_NODE_IMAGE;
        else
            info.node_type = ERGO_UI_NODE_BASE;

        count++;
    }
    return count;
}

void ergo_ui_reparent(ErgoUINodeHandle node, ErgoUINodeHandle new_parent) {
    std::lock_guard lock(g_editor.mutex);
    auto* n = g_ui_hierarchy.find_by_id(node.id);
    auto* p = g_ui_hierarchy.find_by_id(new_parent.id);
    UIHierarchy::reparent(n, p);
}

void ergo_ui_set_sibling_index(ErgoUINodeHandle node, int32_t index) {
    std::lock_guard lock(g_editor.mutex);
    auto* n = g_ui_hierarchy.find_by_id(node.id);
    if (n) n->set_sibling_index(index);
}

} // extern "C"
