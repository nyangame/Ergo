#pragma once

#include "plugin_descriptor.hpp"
#include <vector>
#include <string_view>
#include <cstdint>

// ============================================================
// PluginRegistry
//
// Central registry that holds every PluginDescriptor known to
// the engine. Core components register at startup; user plugins
// can register dynamically (e.g. from a game DLL).
//
// The registry is a global singleton following Ergo's convention
// (same pattern as g_physics, g_resources).
//
// Thread safety: register/unregister must happen before the
// editor tick loop starts. Read-only queries are safe from any
// thread after registration is complete.
// ============================================================

class PluginRegistry {
public:
    // ---------------------------------------------------------
    // Registration
    // ---------------------------------------------------------

    /// Register a plugin descriptor. The descriptor must remain
    /// valid for the lifetime of the registry (typically static).
    /// Returns true if registration succeeded (no duplicate type_name).
    bool register_plugin(const PluginDescriptor* descriptor);

    /// Unregister a plugin by type name.
    /// Returns true if the plugin was found and removed.
    bool unregister_plugin(std::string_view type_name);

    // ---------------------------------------------------------
    // Queries
    // ---------------------------------------------------------

    /// Find a plugin descriptor by its type_name.
    /// Returns nullptr if not found.
    const PluginDescriptor* find(std::string_view type_name) const;

    /// Total number of registered plugins.
    uint32_t count() const;

    /// Get all registered descriptors (iteration order is registration order).
    const std::vector<const PluginDescriptor*>& all() const;

    /// Get plugins filtered by category.
    std::vector<const PluginDescriptor*> by_category(std::string_view category) const;

    /// Get plugins that should appear in the "Add Component" menu.
    std::vector<const PluginDescriptor*> add_menu_plugins() const;

    // ---------------------------------------------------------
    // Lifecycle
    // ---------------------------------------------------------

    /// Remove all registered plugins.
    void clear();

    /// Register all built-in core plugins.
    /// Called once during engine/editor initialization.
    void register_core_plugins();

private:
    std::vector<const PluginDescriptor*> plugins_;
};

// Global plugin registry instance
inline PluginRegistry g_plugin_registry;
