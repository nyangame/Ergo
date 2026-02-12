#pragma once
#include <cmath>

namespace easing {

inline float linear(float t) { return t; }

inline float in_quad(float t) { return t * t; }
inline float out_quad(float t) { return t * (2.0f - t); }
inline float in_out_quad(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

inline float in_cubic(float t) { return t * t * t; }
inline float out_cubic(float t) { float u = t - 1.0f; return u * u * u + 1.0f; }
inline float in_out_cubic(float t) {
    return t < 0.5f
        ? 4.0f * t * t * t
        : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
}

inline float in_quart(float t) { return t * t * t * t; }
inline float out_quart(float t) { float u = t - 1.0f; return 1.0f - u * u * u * u; }
inline float in_out_quart(float t) {
    return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - 8.0f * (t - 1.0f) * (t - 1.0f) * (t - 1.0f) * (t - 1.0f);
}

constexpr float PI = 3.14159265358979323846f;

inline float in_sine(float t) { return 1.0f - std::cos(t * PI * 0.5f); }
inline float out_sine(float t) { return std::sin(t * PI * 0.5f); }
inline float in_out_sine(float t) { return 0.5f * (1.0f - std::cos(PI * t)); }

inline float in_expo(float t) { return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f)); }
inline float out_expo(float t) { return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t); }

inline float in_back(float t) {
    constexpr float s = 1.70158f;
    return t * t * ((s + 1.0f) * t - s);
}
inline float out_back(float t) {
    constexpr float s = 1.70158f;
    float u = t - 1.0f;
    return u * u * ((s + 1.0f) * u + s) + 1.0f;
}

inline float in_elastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return -std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.1f) * 5.0f * PI);
}
inline float out_elastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return std::pow(2.0f, -10.0f * t) * std::sin((t - 0.1f) * 5.0f * PI) + 1.0f;
}

inline float out_bounce(float t) {
    if (t < 1.0f / 2.75f) return 7.5625f * t * t;
    if (t < 2.0f / 2.75f) { t -= 1.5f / 2.75f; return 7.5625f * t * t + 0.75f; }
    if (t < 2.5f / 2.75f) { t -= 2.25f / 2.75f; return 7.5625f * t * t + 0.9375f; }
    t -= 2.625f / 2.75f; return 7.5625f * t * t + 0.984375f;
}
inline float in_bounce(float t) { return 1.0f - out_bounce(1.0f - t); }

} // namespace easing
