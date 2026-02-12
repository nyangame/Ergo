#pragma once
#include "../math/vec3.hpp"
#include "../math/quat.hpp"
#include <string>
#include <vector>
#include <cstdint>

struct Keyframe {
    float time = 0.0f;
    Vec3f position;
    Quat rotation;
    Vec3f scale{1.0f, 1.0f, 1.0f};
};

struct BoneChannel {
    int32_t bone_index = -1;
    std::vector<Keyframe> keyframes;

    // Find surrounding keyframes for interpolation
    std::pair<size_t, size_t> find_keyframes(float time) const {
        if (keyframes.empty()) return {0, 0};
        for (size_t i = 0; i + 1 < keyframes.size(); ++i) {
            if (time < keyframes[i + 1].time) return {i, i + 1};
        }
        return {keyframes.size() - 1, keyframes.size() - 1};
    }
};

struct AnimationClip {
    std::string name;
    float duration = 0.0f;
    std::vector<BoneChannel> channels;
    bool loop = true;
};
