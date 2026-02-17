#include "plugin_registry.hpp"
#include "core_plugins.hpp"
#include <algorithm>
#include <cstring>

// ============================================================
// Registration
// ============================================================

bool PluginRegistry::register_plugin(const PluginDescriptor* descriptor) {
    if (!descriptor || !descriptor->type_name) return false;

    // Reject duplicates
    if (find(descriptor->type_name) != nullptr) return false;

    plugins_.push_back(descriptor);
    return true;
}

bool PluginRegistry::unregister_plugin(std::string_view type_name) {
    auto it = std::remove_if(plugins_.begin(), plugins_.end(),
        [type_name](const PluginDescriptor* d) {
            return type_name == d->type_name;
        });
    if (it == plugins_.end()) return false;
    plugins_.erase(it, plugins_.end());
    return true;
}

// ============================================================
// Queries
// ============================================================

const PluginDescriptor* PluginRegistry::find(std::string_view type_name) const {
    for (const auto* d : plugins_) {
        if (type_name == d->type_name) return d;
    }
    return nullptr;
}

uint32_t PluginRegistry::count() const {
    return static_cast<uint32_t>(plugins_.size());
}

const std::vector<const PluginDescriptor*>& PluginRegistry::all() const {
    return plugins_;
}

std::vector<const PluginDescriptor*> PluginRegistry::by_category(
    std::string_view category) const
{
    std::vector<const PluginDescriptor*> result;
    for (const auto* d : plugins_) {
        if (d->category && category == d->category)
            result.push_back(d);
    }
    return result;
}

std::vector<const PluginDescriptor*> PluginRegistry::add_menu_plugins() const {
    std::vector<const PluginDescriptor*> result;
    for (const auto* d : plugins_) {
        if (d->visible_in_add_menu)
            result.push_back(d);
    }
    return result;
}

// ============================================================
// Lifecycle
// ============================================================

void PluginRegistry::clear() {
    plugins_.clear();
}

void PluginRegistry::register_core_plugins() {
    ergo::register_core_plugins(*this);
}
