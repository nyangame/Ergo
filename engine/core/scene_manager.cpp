#include "scene_manager.hpp"
#include <algorithm>

void SceneManager::change(std::unique_ptr<Scene> scene, float fade_duration) {
    if (fade_duration <= 0.0f) {
        if (!stack_.empty()) {
            stack_.back()->on_exit();
            stack_.pop_back();
        }
        stack_.push_back(std::move(scene));
        stack_.back()->on_enter();
    } else {
        trans_state_ = TransitionState::FadeOut;
        trans_timer_ = 0.0f;
        trans_duration_ = fade_duration;
        trans_action_ = [this, s = std::shared_ptr<Scene>(scene.release())]() mutable {
            if (!stack_.empty()) {
                stack_.back()->on_exit();
                stack_.pop_back();
            }
            stack_.push_back(std::unique_ptr<Scene>(s.get()));
            s.reset();
            stack_.back()->on_enter();
        };
    }
}

void SceneManager::push(std::unique_ptr<Scene> scene) {
    if (!stack_.empty()) {
        stack_.back()->on_pause();
    }
    stack_.push_back(std::move(scene));
    stack_.back()->on_enter();
}

void SceneManager::pop() {
    if (stack_.empty()) return;
    stack_.back()->on_exit();
    stack_.pop_back();
    if (!stack_.empty()) {
        stack_.back()->on_resume();
    }
}

void SceneManager::update(float dt) {
    // Handle transition
    if (trans_state_ != TransitionState::None) {
        trans_timer_ += dt;
        float half = trans_duration_ * 0.5f;

        if (trans_state_ == TransitionState::FadeOut) {
            trans_alpha_ = std::min(1.0f, trans_timer_ / half);
            if (trans_timer_ >= half) {
                if (trans_action_) {
                    trans_action_();
                    trans_action_ = nullptr;
                }
                trans_state_ = TransitionState::FadeIn;
                trans_timer_ = 0.0f;
            }
        } else if (trans_state_ == TransitionState::FadeIn) {
            trans_alpha_ = 1.0f - std::min(1.0f, trans_timer_ / half);
            if (trans_timer_ >= half) {
                trans_state_ = TransitionState::None;
                trans_alpha_ = 0.0f;
            }
        }
    }

    if (!stack_.empty()) {
        stack_.back()->on_update(dt);
    }
}

void SceneManager::draw(RenderContext& ctx) {
    if (!stack_.empty()) {
        stack_.back()->on_draw(ctx);
    }
}

Scene* SceneManager::current() const {
    return stack_.empty() ? nullptr : stack_.back().get();
}
