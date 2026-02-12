#pragma once
#include "skeleton.hpp"
#include "animation_clip.hpp"
#include "../math/mat4.hpp"
#include <vector>
#include <string>
#include <unordered_map>

class AnimationPlayer {
public:
    void set_skeleton(const Skeleton* skeleton);
    void add_clip(const AnimationClip& clip);

    void play(std::string_view clip_name, bool loop = true);
    void stop();
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }

    void update(float dt);

    // Get final bone matrices for GPU upload
    const std::vector<Mat4>& bone_matrices() const { return final_matrices_; }

    float current_time() const { return current_time_; }
    bool is_playing() const { return playing_; }
    std::string_view current_clip_name() const { return current_clip_name_; }

    float blend_factor = 0.0f;
    float playback_speed = 1.0f;

private:
    const Skeleton* skeleton_ = nullptr;
    std::unordered_map<std::string, AnimationClip> clips_;

    std::string current_clip_name_;
    const AnimationClip* current_clip_ = nullptr;
    float current_time_ = 0.0f;
    bool playing_ = false;
    bool paused_ = false;

    std::vector<Mat4> local_transforms_;
    std::vector<Mat4> global_transforms_;
    std::vector<Mat4> final_matrices_;

    void evaluate_clip(const AnimationClip& clip, float time);
    void compute_global_transforms();
};
