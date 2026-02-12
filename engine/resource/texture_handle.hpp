#pragma once
#include <cstdint>

struct TextureHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

struct Rect {
    float x = 0, y = 0, w = 1, h = 1;
};
