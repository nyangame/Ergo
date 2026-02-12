#pragma once
#include <vector>
#include <string>
#include <cstdint>

// Post-process effect base
struct PostProcessEffect {
    std::string name;
    bool enabled = true;

    virtual ~PostProcessEffect() = default;
    virtual void apply() = 0;
};

// Fade effect for scene transitions
struct FadeEffect : PostProcessEffect {
    float alpha = 0.0f;  // 0.0 = transparent, 1.0 = opaque
    uint8_t r = 0, g = 0, b = 0;  // fade color

    FadeEffect() { name = "Fade"; }
    void apply() override {}  // GPU implementation in renderer
};

// Vignette effect
struct VignetteEffect : PostProcessEffect {
    float intensity = 0.5f;
    float smoothness = 0.5f;

    VignetteEffect() { name = "Vignette"; }
    void apply() override {}
};

// Bloom effect
struct BloomEffect : PostProcessEffect {
    float threshold = 1.0f;
    float intensity = 1.0f;
    int blur_passes = 4;

    BloomEffect() { name = "Bloom"; }
    void apply() override {}
};

// Color grading
struct ColorGradeEffect : PostProcessEffect {
    float brightness = 1.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    float gamma = 1.0f;

    ColorGradeEffect() { name = "ColorGrade"; }
    void apply() override {}
};

// Post-process stack manager
class PostProcessStack {
public:
    template<typename T>
    T& add() {
        auto effect = std::make_unique<T>();
        T& ref = *effect;
        effects_.push_back(std::move(effect));
        return ref;
    }

    void remove(const std::string& name) {
        effects_.erase(
            std::remove_if(effects_.begin(), effects_.end(),
                [&](const auto& e) { return e->name == name; }),
            effects_.end());
    }

    void apply_all() {
        for (auto& effect : effects_) {
            if (effect->enabled) effect->apply();
        }
    }

    PostProcessEffect* get(const std::string& name) {
        for (auto& e : effects_) {
            if (e->name == name) return e.get();
        }
        return nullptr;
    }

    size_t effect_count() const { return effects_.size(); }
    void clear() { effects_.clear(); }

private:
    std::vector<std::unique_ptr<PostProcessEffect>> effects_;
};

inline PostProcessStack g_post_process;
