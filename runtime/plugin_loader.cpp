#include "plugin_loader.hpp"
#include <algorithm>
#include <cstdio>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace {

void* open_dll(const char* path) {
#ifdef _WIN32
    return LoadLibraryA(path);
#else
    return dlopen(path, RTLD_LAZY);
#endif
}

void close_dll(void* handle) {
    if (!handle) return;
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

template<typename Fn>
Fn resolve_symbol(void* handle, const char* name) {
#ifdef _WIN32
    return reinterpret_cast<Fn>(GetProcAddress(static_cast<HMODULE>(handle), name));
#else
    return reinterpret_cast<Fn>(dlsym(handle, name));
#endif
}

const char* dll_error() {
#ifdef _WIN32
    static char buf[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
                   0, buf, sizeof(buf), nullptr);
    return buf;
#else
    return dlerror();
#endif
}

} // namespace

// ---------------------------------------------------------------------------
// PluginManager
// ---------------------------------------------------------------------------

uint64_t PluginManager::load(std::string_view dll_path) {
    std::string path_str(dll_path);

    void* handle = open_dll(path_str.c_str());
    if (!handle) {
        std::fprintf(stderr, "[Ergo] Failed to load plugin DLL: %s (%s)\n",
                     path_str.c_str(), dll_error());
        return 0;
    }

    auto get_info = resolve_symbol<ErgoPluginInfo*(*)()>(
        handle, "ergo_get_plugin_info");
    auto get_callbacks = resolve_symbol<ErgoPluginCallbacks*(*)()>(
        handle, "ergo_get_plugin_callbacks");

    if (!get_info) {
        std::fprintf(stderr, "[Ergo] ergo_get_plugin_info not found in: %s\n",
                     path_str.c_str());
        close_dll(handle);
        return 0;
    }
    if (!get_callbacks) {
        std::fprintf(stderr, "[Ergo] ergo_get_plugin_callbacks not found in: %s\n",
                     path_str.c_str());
        close_dll(handle);
        return 0;
    }

    ErgoPluginInfo* info = get_info();
    ErgoPluginCallbacks* callbacks = get_callbacks();

    if (!info || !callbacks) {
        std::fprintf(stderr, "[Ergo] Plugin returned null info or callbacks: %s\n",
                     path_str.c_str());
        close_dll(handle);
        return 0;
    }

    uint64_t id = next_id_++;
    plugins_.push_back({id, handle, info, callbacks, std::move(path_str)});

    std::fprintf(stderr, "[Ergo] Plugin loaded: %s v%s (%s)\n",
                 info->name ? info->name : "(unnamed)",
                 info->version ? info->version : "?",
                 dll_path.data());

    return id;
}

bool PluginManager::unload(uint64_t id) {
    auto it = std::find_if(plugins_.begin(), plugins_.end(),
                           [id](const PluginDLL& p) { return p.id == id; });
    if (it == plugins_.end()) return false;

    if (it->callbacks && it->callbacks->on_shutdown) {
        it->callbacks->on_shutdown();
    }
    close_dll(it->handle);
    plugins_.erase(it);
    return true;
}

void PluginManager::unload_all() {
    // Reverse order: last loaded first unloaded
    for (auto it = plugins_.rbegin(); it != plugins_.rend(); ++it) {
        if (it->callbacks && it->callbacks->on_shutdown) {
            it->callbacks->on_shutdown();
        }
        close_dll(it->handle);
    }
    plugins_.clear();
}

void PluginManager::init_all(const ErgoEngineAPI* api) {
    for (auto& p : plugins_) {
        if (p.callbacks && p.callbacks->on_init) {
            p.callbacks->on_init(api);
        }
    }
}

void PluginManager::update_all(float dt) {
    for (auto& p : plugins_) {
        if (p.callbacks && p.callbacks->on_update) {
            p.callbacks->on_update(dt);
        }
    }
}

void PluginManager::draw_all() {
    for (auto& p : plugins_) {
        if (p.callbacks && p.callbacks->on_draw) {
            p.callbacks->on_draw();
        }
    }
}

void PluginManager::shutdown_all() {
    // shutdown callbacks only (without closing DLLs)
    for (auto& p : plugins_) {
        if (p.callbacks && p.callbacks->on_shutdown) {
            p.callbacks->on_shutdown();
        }
    }
}

const PluginDLL* PluginManager::get(uint64_t id) const {
    for (auto& p : plugins_) {
        if (p.id == id) return &p;
    }
    return nullptr;
}
