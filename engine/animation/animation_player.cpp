#include "animation_player.hpp"
#include <algorithm>
#include <cmath>

void AnimationPlayer::set_skeleton(const Skeleton* skeleton) {
    skeleton_ = skeleton;
    if (skeleton_) {
        size_t n = skeleton_->bone_count();
        local_transforms_.resize(n);
        global_transforms_.resize(n);
        final_matrices_.resize(n);
        for (size_t i = 0; i < n; ++i) {
            local_transforms_[i] = skeleton_->bones[i].local_bind_pose;
        }
    }
}

void AnimationPlayer::add_clip(const AnimationClip& clip) {
    clips_[clip.name] = clip;
}

void AnimationPlayer::play(std::string_view clip_name, bool loop) {
    auto it = clips_.find(std::string(clip_name));
    if (it == clips_.end()) return;
    it->second.loop = loop;
    current_clip_ = &it->second;
    current_clip_name_ = clip_name;
    current_time_ = 0.0f;
    playing_ = true;
    paused_ = false;
}

void AnimationPlayer::stop() {
    playing_ = false;
    current_time_ = 0.0f;
    current_clip_ = nullptr;
    current_clip_name_.clear();
}

void AnimationPlayer::update(float dt) {
    if (!playing_ || !current_clip_ || !skeleton_ || paused_) return;

    current_time_ += dt * playback_speed;

    if (current_time_ >= current_clip_->duration) {
        if (current_clip_->loop) {
            current_time_ = std::fmod(current_time_, current_clip_->duration);
        } else {
            current_time_ = current_clip_->duration;
            playing_ = false;
        }
    }

    evaluate_clip(*current_clip_, current_time_);
    compute_global_transforms();
}

void AnimationPlayer::evaluate_clip(const AnimationClip& clip, float time) {
    if (!skeleton_) return;

    // Reset to bind pose
    for (size_t i = 0; i < skeleton_->bone_count(); ++i) {
        local_transforms_[i] = skeleton_->bones[i].local_bind_pose;
    }

    // Apply animation channels
    for (const auto& channel : clip.channels) {
        if (channel.bone_index < 0 ||
            channel.bone_index >= static_cast<int32_t>(skeleton_->bone_count())) continue;
        if (channel.keyframes.empty()) continue;

        auto [idx0, idx1] = channel.find_keyframes(time);
        const auto& kf0 = channel.keyframes[idx0];
        const auto& kf1 = channel.keyframes[idx1];

        float t = 0.0f;
        if (idx0 != idx1 && kf1.time > kf0.time) {
            t = (time - kf0.time) / (kf1.time - kf0.time);
        }

        // Interpolate position
        Vec3f pos = kf0.position + (kf1.position - kf0.position) * t;
        // Interpolate rotation (slerp)
        Quat rot = Quat::slerp(kf0.rotation, kf1.rotation, t);
        // Interpolate scale
        Vec3f scl = kf0.scale + (kf1.scale - kf0.scale) * t;

        // Compose local transform
        Mat4 tm = Mat4::translation(pos) * rot.to_mat4() * Mat4::scale(scl);
        local_transforms_[channel.bone_index] = tm;
    }
}

void AnimationPlayer::compute_global_transforms() {
    if (!skeleton_) return;

    for (size_t i = 0; i < skeleton_->bone_count(); ++i) {
        int32_t parent = skeleton_->bones[i].parent_index;
        if (parent >= 0) {
            global_transforms_[i] = global_transforms_[parent] * local_transforms_[i];
        } else {
            global_transforms_[i] = local_transforms_[i];
        }
        final_matrices_[i] = global_transforms_[i] * skeleton_->bones[i].inverse_bind_pose;
    }
}
