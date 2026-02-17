#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <functional>
#include "../concepts.hpp"

struct GameObject;

// BehaviourLike concept is defined in concepts.hpp (included above)

// ============================================================
// IBehaviour: type-erased interface (same pattern as ITask)
// ============================================================

struct PluginDescriptor;  // defined in engine/plugin/plugin_descriptor.hpp

struct IBehaviour {
    virtual ~IBehaviour() = default;
    virtual void start() = 0;
    virtual void update(float dt) = 0;
    virtual void draw(RenderContext& ctx) = 0;
    virtual void release() = 0;
    virtual bool has_draw() const = 0;
    virtual std::string_view type_name() const = 0;
    virtual std::type_index type_id() const = 0;
    virtual void* raw_ptr() = 0;

    // Threading introspection
    virtual ThreadingPolicy threading_policy() const = 0;
    virtual bool is_thread_aware() const = 0;

    // Plugin introspection
    virtual bool has_plugin_descriptor() const = 0;
    virtual const PluginDescriptor* plugin_descriptor() const = 0;
};

// ============================================================
// BehaviourModel: concept-constrained wrapper (mirrors TaskModel)
// ============================================================

template<BehaviourLike T>
struct BehaviourModel final : IBehaviour {
    T behaviour;

    template<typename... Args>
    explicit BehaviourModel(Args&&... args)
        : behaviour(std::forward<Args>(args)...) {}

    void start() override { behaviour.start(); }
    void update(float dt) override { behaviour.update(dt); }

    void draw(RenderContext& ctx) override {
        if constexpr (Drawable<T, RenderContext>)
            behaviour.draw(ctx);
    }

    void release() override { behaviour.release(); }

    bool has_draw() const override {
        return Drawable<T, RenderContext>;
    }

    std::string_view type_name() const override {
        return T::type_name();
    }

    std::type_index type_id() const override {
        return std::type_index(typeid(T));
    }

    void* raw_ptr() override { return &behaviour; }

    ThreadingPolicy threading_policy() const override {
        if constexpr (ThreadAware<T>)
            return T::threading_policy();
        else
            return ThreadingPolicy::MainThread;  // safe default
    }

    bool is_thread_aware() const override {
        return ThreadAware<T>;
    }

    bool has_plugin_descriptor() const override {
        return PluginLike<T>;
    }

    const PluginDescriptor* plugin_descriptor() const override {
        if constexpr (PluginLike<T>)
            return &T::plugin_descriptor();
        else
            return nullptr;
    }
};

// ============================================================
// BehaviourHolder: attaches to a GameObject, owns behaviours
// ============================================================

class BehaviourHolder {
    std::vector<std::unique_ptr<IBehaviour>> behaviours_;
    bool started_ = false;

public:
    template<BehaviourLike T, typename... Args>
    T& add(Args&&... args) {
        auto model = std::make_unique<BehaviourModel<T>>(std::forward<Args>(args)...);
        T& ref = model->behaviour;
        behaviours_.push_back(std::move(model));
        if (started_) {
            behaviours_.back()->start();
        }
        return ref;
    }

    template<typename T>
    T* get() {
        auto idx = std::type_index(typeid(T));
        for (auto& b : behaviours_) {
            if (b->type_id() == idx)
                return static_cast<T*>(b->raw_ptr());
        }
        return nullptr;
    }

    template<typename T>
    const T* get() const {
        auto idx = std::type_index(typeid(T));
        for (auto& b : behaviours_) {
            if (b->type_id() == idx)
                return static_cast<const T*>(b->raw_ptr());
        }
        return nullptr;
    }

    template<typename T>
    bool has() const {
        auto idx = std::type_index(typeid(T));
        for (auto& b : behaviours_) {
            if (b->type_id() == idx) return true;
        }
        return false;
    }

    void remove(std::string_view name) {
        behaviours_.erase(
            std::remove_if(behaviours_.begin(), behaviours_.end(),
                [name](const auto& b) { return b->type_name() == name; }),
            behaviours_.end());
    }

    void start() {
        started_ = true;
        for (auto& b : behaviours_)
            b->start();
    }

    void update(float dt) {
        for (auto& b : behaviours_)
            b->update(dt);
    }

    void draw(RenderContext& ctx) {
        for (auto& b : behaviours_) {
            if (b->has_draw())
                b->draw(ctx);
        }
    }

    void release() {
        for (auto& b : behaviours_)
            b->release();
        behaviours_.clear();
        started_ = false;
    }

    size_t count() const { return behaviours_.size(); }

    // Iteration for GUI inspector
    void for_each(std::function<void(IBehaviour&)> fn) {
        for (auto& b : behaviours_)
            fn(*b);
    }

    void for_each(std::function<void(const IBehaviour&)> fn) const {
        for (auto& b : behaviours_)
            fn(*b);
    }

    // Threading introspection for this holder's behaviours
    struct BehaviourThreadingInfo {
        std::string_view name;
        ThreadingPolicy policy;
        bool thread_aware;
    };

    std::vector<BehaviourThreadingInfo> threading_report() const {
        std::vector<BehaviourThreadingInfo> report;
        report.reserve(behaviours_.size());
        for (const auto& b : behaviours_) {
            report.push_back({
                b->type_name(),
                b->threading_policy(),
                b->is_thread_aware()
            });
        }
        return report;
    }

    // Check if all behaviours in this holder can run off the main thread
    bool all_parallelizable() const {
        for (const auto& b : behaviours_) {
            if (b->threading_policy() == ThreadingPolicy::MainThread)
                return false;
        }
        return !behaviours_.empty();
    }
};
