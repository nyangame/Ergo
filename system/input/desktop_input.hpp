#pragma once
#include <cstdint>
#include "engine/math/vec2.hpp"

// Replaces CppSampleGame's CheckHitKey, GetMousePoint
class DesktopInput {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    DesktopInput();
    ~DesktopInput();

    // Satisfies InputBackend concept
    void poll_events();
    bool is_key_down(uint32_t key) const;
    bool is_key_pressed(uint32_t key) const;
    Vec2f mouse_position() const;
    bool is_mouse_button_down(uint32_t button) const;
};
