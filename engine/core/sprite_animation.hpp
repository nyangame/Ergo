#pragma once
#include "../resource/texture_handle.hpp"
#include <vector>
#include <string>
#include <unordered_map>

struct AnimationFrame {
    Rect uv;
    float duration = 0.1f;
};

struct SpriteAnimation {
    TextureHandle texture;
    std::vector<AnimationFrame> frames;
    bool loop = true;

    // Playback state
    float timer = 0.0f;
    uint32_t current_frame = 0;
    bool finished = false;

    void update(float dt) {
        if (finished || frames.empty()) return;
        timer += dt;
        while (timer >= frames[current_frame].duration) {
            timer -= frames[current_frame].duration;
            ++current_frame;
            if (current_frame >= static_cast<uint32_t>(frames.size())) {
                if (loop) {
                    current_frame = 0;
                } else {
                    current_frame = static_cast<uint32_t>(frames.size()) - 1;
                    finished = true;
                    return;
                }
            }
        }
    }

    const Rect& current_uv() const {
        return frames[current_frame].uv;
    }

    void reset() {
        timer = 0.0f;
        current_frame = 0;
        finished = false;
    }

    // Create from spritesheet grid
    static SpriteAnimation from_grid(TextureHandle tex, int cols, int rows,
                                      int total_frames, float frame_duration) {
        SpriteAnimation anim;
        anim.texture = tex;
        float fw = 1.0f / cols;
        float fh = 1.0f / rows;
        for (int i = 0; i < total_frames; ++i) {
            int col = i % cols;
            int row = i / cols;
            AnimationFrame f;
            f.uv = {col * fw, row * fh, fw, fh};
            f.duration = frame_duration;
            anim.frames.push_back(f);
        }
        return anim;
    }
};

struct AnimationController {
    std::unordered_map<std::string, SpriteAnimation> animations;
    std::string current_name;

    void play(std::string_view name) {
        std::string key(name);
        if (key == current_name) return;
        auto it = animations.find(key);
        if (it != animations.end()) {
            current_name = key;
            it->second.reset();
        }
    }

    void update(float dt) {
        auto it = animations.find(current_name);
        if (it != animations.end()) {
            it->second.update(dt);
        }
    }

    const SpriteAnimation* current() const {
        auto it = animations.find(current_name);
        return (it != animations.end()) ? &it->second : nullptr;
    }
};
