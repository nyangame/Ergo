#pragma once
#include <vector>
#include <memory>
#include <functional>

// Forward declaration
struct RenderContext;

struct Scene {
    virtual ~Scene() = default;
    virtual void on_enter() = 0;
    virtual void on_exit() = 0;
    virtual void on_update(float dt) = 0;
    virtual void on_draw(RenderContext& ctx) = 0;
    virtual void on_pause() {}
    virtual void on_resume() {}
};

class SceneManager {
public:
    // Replace current scene
    void change(std::unique_ptr<Scene> scene, float fade_duration = 0.0f);

    // Push scene onto stack (pause menu, etc.)
    void push(std::unique_ptr<Scene> scene);

    // Pop top scene from stack
    void pop();

    void update(float dt);
    void draw(RenderContext& ctx);

    Scene* current() const;
    size_t stack_size() const { return stack_.size(); }
    bool empty() const { return stack_.empty(); }

private:
    enum class TransitionState { None, FadeOut, FadeIn };

    std::vector<std::unique_ptr<Scene>> stack_;

    TransitionState trans_state_ = TransitionState::None;
    float trans_timer_ = 0.0f;
    float trans_duration_ = 0.5f;
    float trans_alpha_ = 0.0f;
    std::function<void()> trans_action_;
};
