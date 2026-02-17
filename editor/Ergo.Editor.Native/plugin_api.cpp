#include "plugin_api.h"
#include "engine/plugin/plugin_registry.hpp"
#include "engine/plugin/plugin_descriptor.hpp"
#include "engine/core/game_object.hpp"
#include "engine/core/behaviour/behaviour.hpp"

#include <cstring>
#include <mutex>

// The editor state mutex (defined in editor_api.cpp)
namespace {
    extern std::mutex& editor_mutex();
}

// ============================================================
// Helper: find a behaviour by type_name on a game object
// ============================================================

namespace {

// Access the editor's object map (defined in editor_api.cpp)
extern GameObject* find_editor_object(uint64_t id);

// Find a PropertyDescriptor in a PluginDescriptor by property name
const PropertyDescriptor* find_property(
    const PluginDescriptor* plugin, const char* property_name)
{
    if (!plugin || !property_name) return nullptr;
    for (uint32_t i = 0; i < plugin->property_count; ++i) {
        if (std::strcmp(plugin->properties[i].name, property_name) == 0)
            return &plugin->properties[i];
    }
    return nullptr;
}

// Convert PropertyType to ErgoPluginPropertyType
ErgoPluginPropertyType to_c_prop_type(PropertyType t) {
    switch (t) {
        case PropertyType::Float:  return ERGO_PLUGIN_PROP_FLOAT;
        case PropertyType::Int:    return ERGO_PLUGIN_PROP_INT;
        case PropertyType::Bool:   return ERGO_PLUGIN_PROP_BOOL;
        case PropertyType::Vec2:   return ERGO_PLUGIN_PROP_VEC2;
        case PropertyType::Vec3:   return ERGO_PLUGIN_PROP_VEC3;
        case PropertyType::Color:  return ERGO_PLUGIN_PROP_COLOR;
        case PropertyType::String: return ERGO_PLUGIN_PROP_STRING;
        case PropertyType::Enum:   return ERGO_PLUGIN_PROP_ENUM;
        case PropertyType::Asset:  return ERGO_PLUGIN_PROP_ASSET;
    }
    return ERGO_PLUGIN_PROP_FLOAT;
}

void fill_plugin_info(const PluginDescriptor* d, ErgoPluginInfo& out) {
    out.type_name          = d->type_name;
    out.display_name       = d->display_name;
    out.category           = d->category;
    out.description        = d->description;
    out.property_count     = d->property_count;
    out.allow_multiple     = d->allow_multiple ? 1 : 0;
    out.removable          = d->removable ? 1 : 0;
    out.visible_in_add_menu = d->visible_in_add_menu ? 1 : 0;
}

} // anonymous namespace

// ============================================================
// Plugin registry queries
// ============================================================

extern "C" {

uint32_t ergo_plugin_get_count(void) {
    return g_plugin_registry.count();
}

uint32_t ergo_plugin_get_all(
    ErgoPluginInfo* out_infos, uint32_t max_count)
{
    const auto& all = g_plugin_registry.all();
    uint32_t count = 0;
    for (const auto* d : all) {
        if (count >= max_count) break;
        fill_plugin_info(d, out_infos[count]);
        count++;
    }
    return count;
}

int32_t ergo_plugin_get_info(
    const char* type_name, ErgoPluginInfo* out_info)
{
    if (!type_name || !out_info) return 0;
    const auto* d = g_plugin_registry.find(type_name);
    if (!d) return 0;
    fill_plugin_info(d, *out_info);
    return 1;
}

uint32_t ergo_plugin_get_by_category(
    const char* category,
    ErgoPluginInfo* out_infos, uint32_t max_count)
{
    if (!category || !out_infos) return 0;
    auto plugins = g_plugin_registry.by_category(category);
    uint32_t count = 0;
    for (const auto* d : plugins) {
        if (count >= max_count) break;
        fill_plugin_info(d, out_infos[count]);
        count++;
    }
    return count;
}

// ============================================================
// Plugin property queries
// ============================================================

uint32_t ergo_plugin_get_properties(
    const char* type_name,
    ErgoPluginPropertyInfo* out_props, uint32_t max_count)
{
    if (!type_name || !out_props) return 0;
    const auto* d = g_plugin_registry.find(type_name);
    if (!d) return 0;

    uint32_t count = 0;
    for (uint32_t i = 0; i < d->property_count && count < max_count; ++i) {
        const auto& p = d->properties[i];
        auto& out = out_props[count];
        out.name             = p.name;
        out.display_name     = p.display_name;
        out.type             = to_c_prop_type(p.type);
        out.range_min        = p.range_min;
        out.range_max        = p.range_max;
        out.range_step       = p.range_step;
        out.has_range        = p.has_range ? 1 : 0;
        out.tooltip          = p.tooltip;
        out.enum_entry_count = p.enum_entry_count;
        count++;
    }
    return count;
}

uint32_t ergo_plugin_get_enum_entries(
    const char* type_name,
    const char* property_name,
    ErgoPluginEnumEntry* out_entries, uint32_t max_count)
{
    if (!type_name || !property_name || !out_entries) return 0;
    const auto* d = g_plugin_registry.find(type_name);
    if (!d) return 0;

    const auto* prop = find_property(d, property_name);
    if (!prop || prop->type != PropertyType::Enum) return 0;

    uint32_t count = 0;
    for (uint32_t i = 0; i < prop->enum_entry_count && count < max_count; ++i) {
        out_entries[count].label = prop->enum_entries[i].label;
        out_entries[count].value = prop->enum_entries[i].value;
        count++;
    }
    return count;
}

// ============================================================
// Plugin property read/write on live behaviour instances
// ============================================================

int32_t ergo_plugin_get_float(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    float* out_value)
{
    if (!type_name || !property_name || !out_value) return 0;

    const auto* plugin = g_plugin_registry.find(type_name);
    if (!plugin) return 0;

    const auto* prop = find_property(plugin, property_name);
    if (!prop || !prop->get) return 0;

    auto* obj = find_editor_object(object.id);
    if (!obj) return 0;

    // Find the behaviour by type_name on the object's components
    // For now, search through the component map using type_name matching
    // This requires the object to have a BehaviourHolder
    auto* holder = obj->get_component<BehaviourHolder>();
    if (!holder) return 0;

    // Iterate behaviours to find matching type
    bool found = false;
    holder->for_each([&](IBehaviour& b) {
        if (found) return;
        if (b.type_name() == std::string_view(type_name)) {
            prop->get(b.raw_ptr(), out_value);
            found = true;
        }
    });
    return found ? 1 : 0;
}

int32_t ergo_plugin_set_float(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    float value)
{
    if (!type_name || !property_name) return 0;

    const auto* plugin = g_plugin_registry.find(type_name);
    if (!plugin) return 0;

    const auto* prop = find_property(plugin, property_name);
    if (!prop || !prop->set) return 0;

    auto* obj = find_editor_object(object.id);
    if (!obj) return 0;

    auto* holder = obj->get_component<BehaviourHolder>();
    if (!holder) return 0;

    bool found = false;
    holder->for_each([&](IBehaviour& b) {
        if (found) return;
        if (b.type_name() == std::string_view(type_name)) {
            prop->set(b.raw_ptr(), &value);
            found = true;
        }
    });
    return found ? 1 : 0;
}

int32_t ergo_plugin_get_int(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    int32_t* out_value)
{
    if (!type_name || !property_name || !out_value) return 0;

    const auto* plugin = g_plugin_registry.find(type_name);
    if (!plugin) return 0;

    const auto* prop = find_property(plugin, property_name);
    if (!prop || !prop->get) return 0;

    auto* obj = find_editor_object(object.id);
    if (!obj) return 0;

    auto* holder = obj->get_component<BehaviourHolder>();
    if (!holder) return 0;

    bool found = false;
    holder->for_each([&](IBehaviour& b) {
        if (found) return;
        if (b.type_name() == std::string_view(type_name)) {
            prop->get(b.raw_ptr(), out_value);
            found = true;
        }
    });
    return found ? 1 : 0;
}

int32_t ergo_plugin_set_int(
    ErgoGameObjectHandle object,
    const char* type_name,
    const char* property_name,
    int32_t value)
{
    if (!type_name || !property_name) return 0;

    const auto* plugin = g_plugin_registry.find(type_name);
    if (!plugin) return 0;

    const auto* prop = find_property(plugin, property_name);
    if (!prop || !prop->set) return 0;

    auto* obj = find_editor_object(object.id);
    if (!obj) return 0;

    auto* holder = obj->get_component<BehaviourHolder>();
    if (!holder) return 0;

    bool found = false;
    holder->for_each([&](IBehaviour& b) {
        if (found) return;
        if (b.type_name() == std::string_view(type_name)) {
            prop->set(b.raw_ptr(), &value);
            found = true;
        }
    });
    return found ? 1 : 0;
}

// ============================================================
// Plugin instance management
// ============================================================

int32_t ergo_plugin_add_to_object(
    ErgoGameObjectHandle object,
    const char* type_name)
{
    if (!type_name) return 0;

    const auto* plugin = g_plugin_registry.find(type_name);
    if (!plugin || !plugin->create_default) return 0;

    auto* obj = find_editor_object(object.id);
    if (!obj) return 0;

    // Ensure the object has a BehaviourHolder
    if (!obj->get_component<BehaviourHolder>()) {
        obj->add_component(BehaviourHolder{});
    }

    auto* holder = obj->get_component<BehaviourHolder>();
    if (!holder) return 0;

    // Check allow_multiple constraint
    if (!plugin->allow_multiple) {
        bool already_exists = false;
        holder->for_each([&](const IBehaviour& b) {
            if (b.type_name() == std::string_view(type_name))
                already_exists = true;
        });
        if (already_exists) return 0;
    }

    plugin->create_default(*holder);
    return 1;
}

int32_t ergo_plugin_remove_from_object(
    ErgoGameObjectHandle object,
    const char* type_name)
{
    if (!type_name) return 0;

    const auto* plugin = g_plugin_registry.find(type_name);
    if (!plugin || !plugin->removable) return 0;

    auto* obj = find_editor_object(object.id);
    if (!obj) return 0;

    auto* holder = obj->get_component<BehaviourHolder>();
    if (!holder) return 0;

    holder->remove(type_name);
    return 1;
}

int32_t ergo_plugin_object_has(
    ErgoGameObjectHandle object,
    const char* type_name)
{
    if (!type_name) return 0;

    auto* obj = find_editor_object(object.id);
    if (!obj) return 0;

    auto* holder = obj->get_component<BehaviourHolder>();
    if (!holder) return 0;

    bool found = false;
    holder->for_each([&](const IBehaviour& b) {
        if (b.type_name() == std::string_view(type_name))
            found = true;
    });
    return found ? 1 : 0;
}

} // extern "C"
