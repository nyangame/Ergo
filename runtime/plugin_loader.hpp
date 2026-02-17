#pragma once
#include "game_interface/plugin_interface.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// A single loaded plugin DLL
struct PluginDLL {
    uint64_t            id       = 0;
    void*               handle   = nullptr;
    ErgoPluginInfo*      info     = nullptr;
    ErgoPluginCallbacks* callbacks = nullptr;
    std::string         path;

    bool valid() const { return handle && info && callbacks; }
};

// Manages multiple plugin DLLs
class PluginManager {
public:
    // Load a plugin DLL and return its handle id (0 on failure)
    uint64_t load(std::string_view dll_path);

    // Unload a plugin by id. Returns true on success.
    bool unload(uint64_t id);

    // Unload all plugins (in reverse load order)
    void unload_all();

    // Lifecycle: forward calls to every loaded plugin
    void init_all(const ErgoEngineAPI* api);
    void update_all(float dt);
    void draw_all();
    void shutdown_all();

    // Query
    uint32_t             count()   const { return static_cast<uint32_t>(plugins_.size()); }
    const PluginDLL*     get(uint64_t id) const;
    const std::vector<PluginDLL>& plugins() const { return plugins_; }

private:
    uint64_t next_id_ = 1;
    std::vector<PluginDLL> plugins_;
};
