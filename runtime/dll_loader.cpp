#include "dll_loader.hpp"
#include <cstdio>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

GameDLL load_game_dll(std::string_view path) {
    GameDLL dll{};
    std::string path_str(path);

#ifdef _WIN32
    dll.handle = LoadLibraryA(path_str.c_str());
    if (!dll.handle) {
        std::fprintf(stderr, "[Ergo] Failed to load game DLL: %s\n", path_str.c_str());
        return dll;
    }
    auto get_callbacks = reinterpret_cast<ErgoGameCallbacks*(*)()>(
        GetProcAddress(static_cast<HMODULE>(dll.handle), "ergo_get_game_callbacks"));
#else
    dll.handle = dlopen(path_str.c_str(), RTLD_LAZY);
    if (!dll.handle) {
        std::fprintf(stderr, "[Ergo] Failed to load game DLL: %s (%s)\n",
                     path_str.c_str(), dlerror());
        return dll;
    }
    auto get_callbacks = reinterpret_cast<ErgoGameCallbacks*(*)()>(
        dlsym(dll.handle, "ergo_get_game_callbacks"));
#endif

    if (!get_callbacks) {
        std::fprintf(stderr, "[Ergo] ergo_get_game_callbacks not found in: %s\n",
                     path_str.c_str());
        unload_game_dll(dll);
        return dll;
    }

    dll.callbacks = get_callbacks();
    if (!dll.callbacks) {
        std::fprintf(stderr, "[Ergo] ergo_get_game_callbacks returned null\n");
        unload_game_dll(dll);
    }

    return dll;
}

void unload_game_dll(GameDLL& dll) {
    if (dll.handle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(dll.handle));
#else
        dlclose(dll.handle);
#endif
        dll.handle = nullptr;
    }
    dll.callbacks = nullptr;
}
