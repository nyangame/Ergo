#pragma once
#include "easing.hpp"
#include <vector>
#include <functional>
#include <algorithm>

using EasingFunc = float(*)(float);

struct Tween {
    float* target = nullptr;
    float start_value = 0.0f;
    float end_value = 0.0f;
    float duration = 0.0f;
    float elapsed = 0.0f;
    EasingFunc ease = easing::linear;
    std::function<void()> on_complete;
    bool finished = false;

    void update(float dt) {
        if (finished) return;
        elapsed += dt;
        float t = std::clamp(elapsed / duration, 0.0f, 1.0f);
        float eased = ease(t);
        if (target) {
            *target = start_value + (end_value - start_value) * eased;
        }
        if (elapsed >= duration) {
            finished = true;
            if (target) *target = end_value;
            if (on_complete) on_complete();
        }
    }
};

class TweenManager {
    std::vector<Tween> tweens_;

public:
    Tween& add(float* target, float from, float to, float duration,
               EasingFunc ease = easing::linear) {
        tweens_.push_back({target, from, to, duration, 0.0f, ease, nullptr, false});
        if (target) *target = from;
        return tweens_.back();
    }

    void update(float dt) {
        for (auto& tw : tweens_) {
            tw.update(dt);
        }
        // Remove finished tweens
        tweens_.erase(
            std::remove_if(tweens_.begin(), tweens_.end(),
                           [](const Tween& t) { return t.finished; }),
            tweens_.end());
    }

    void clear() {
        tweens_.clear();
    }

    size_t active_count() const { return tweens_.size(); }
};

inline TweenManager g_tweens;
