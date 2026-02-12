#pragma once
#include "game_interface/game_interface.h"
#include <string_view>

// Game DLL loader
// Dynamically loads a game shared library and retrieves its callbacks
struct GameDLL {
    void* handle = nullptr;
    ErgoGameCallbacks* callbacks = nullptr;

    bool valid() const { return handle != nullptr && callbacks != nullptr; }
};

GameDLL load_game_dll(std::string_view path);
void unload_game_dll(GameDLL& dll);
