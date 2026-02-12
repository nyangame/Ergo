#pragma once
#include "engine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Engine API provided to the game DLL
typedef struct {
    // Drawing (replaces CppSampleGame's DrawBox, DrawCircle, etc.)
    void (*draw_rect)(ErgoVec2 pos, ErgoSize2 size, ErgoColor color, int filled);
    void (*draw_circle)(ErgoVec2 center, float radius, ErgoColor color, int filled);
    void (*draw_text)(ErgoVec2 pos, const char* text, ErgoColor color, float scale);

    // Input (replaces CppSampleGame's CheckHitKey, GetMousePoint)
    int (*is_key_down)(uint32_t key);
    int (*is_key_pressed)(uint32_t key);
    ErgoVec2 (*mouse_position)(void);

    // Resources
    ErgoTextureHandle (*load_texture)(const char* path);
    void (*unload_texture)(ErgoTextureHandle handle);
} ErgoEngineAPI;

// Callbacks exported by the game DLL
typedef struct {
    void (*on_init)(const ErgoEngineAPI* api);
    void (*on_update)(float dt);
    void (*on_draw)(void);
    void (*on_shutdown)(void);
} ErgoGameCallbacks;

// DLL entry point (implemented by the game DLL)
#ifdef _WIN32
    #define ERGO_EXPORT __declspec(dllexport)
#else
    #define ERGO_EXPORT __attribute__((visibility("default")))
#endif

ERGO_EXPORT ErgoGameCallbacks* ergo_get_game_callbacks(void);

#ifdef __cplusplus
}
#endif
