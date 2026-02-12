#pragma once
#include <cstdint>
#include <vector>
#include "engine/math/vec2.hpp"

struct TouchPoint {
    uint32_t id;
    Vec2f position;
    bool active;
};

// Android/iOS touch input
class TouchInput {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    TouchInput();
    ~TouchInput();

    // Satisfies InputBackend concept
    void poll_events();
    bool is_key_down(uint32_t key) const;      // Virtual key
    bool is_key_pressed(uint32_t key) const;
    Vec2f mouse_position() const;               // Primary touch position

    // Touch-specific
    const std::vector<TouchPoint>& touches() const;
};
