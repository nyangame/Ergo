#pragma once
#include "engine_types.h"
#include "game_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// Plugin metadata returned by the DLL
// ---------------------------------------------------------------------------
typedef struct {
    const char* name;
    const char* version;
    const char* description;
    const char* author;
} ErgoPluginInfo;

// ---------------------------------------------------------------------------
// Plugin callbacks (same lifecycle as game DLL)
// ---------------------------------------------------------------------------
typedef struct {
    void (*on_init)(const ErgoEngineAPI* api);
    void (*on_update)(float dt);
    void (*on_draw)(void);
    void (*on_shutdown)(void);
} ErgoPluginCallbacks;

// ---------------------------------------------------------------------------
// DLL entry points (implemented by each plugin DLL)
// ---------------------------------------------------------------------------
ERGO_EXPORT ErgoPluginInfo*      ergo_get_plugin_info(void);
ERGO_EXPORT ErgoPluginCallbacks* ergo_get_plugin_callbacks(void);

#ifdef __cplusplus
}
#endif
