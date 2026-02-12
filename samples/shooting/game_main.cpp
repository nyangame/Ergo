#include "game_interface/game_interface.h"
#include "engine/core/state_machine.hpp"
#include "scenes/title_scene.hpp"
#include "scenes/ingame_scene.hpp"

// Game state
static const ErgoEngineAPI* s_api = nullptr;
static StateMachine<TitleScene, InGameScene>* s_sequence = nullptr;

static void game_on_init(const ErgoEngineAPI* api) {
    s_api = api;
    s_sequence = new StateMachine<TitleScene, InGameScene>();
    s_sequence->transition<TitleScene>();
}

static void game_on_update(float dt) {
    if (s_sequence) {
        s_sequence->update(dt);
    }
}

static void game_on_draw() {
    // Drawing is handled through task system and scene draw
}

static void game_on_shutdown() {
    delete s_sequence;
    s_sequence = nullptr;
    s_api = nullptr;
}

static ErgoGameCallbacks s_callbacks = {
    game_on_init,
    game_on_update,
    game_on_draw,
    game_on_shutdown
};

extern "C" {

ERGO_EXPORT ErgoGameCallbacks* ergo_get_game_callbacks() {
    return &s_callbacks;
}

}
