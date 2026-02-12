#pragma once

struct Size2f {
    float w = 0.0f;
    float h = 0.0f;

    constexpr Size2f() = default;
    constexpr Size2f(float w, float h) : w(w), h(h) {}

    constexpr float half_w() const { return w * 0.5f; }
    constexpr float half_h() const { return h * 0.5f; }
    constexpr float radius() const { return w * 0.5f; }
};
