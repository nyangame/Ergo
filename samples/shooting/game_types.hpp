#pragma once
#include <cstdint>

// Corresponds to CppSampleGame's GameType.h
enum class GameObjectType : uint32_t {
    Invalid = 0,
    Player,
    Enemy,
    Bullet
};
