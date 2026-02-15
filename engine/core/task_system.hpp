#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include "concepts.hpp"

struct RenderContext; // Forward declaration

struct TaskHandle {
    uint64_t id = 0;
    bool valid() const { return id != 0; }
};

enum class TaskLayer : uint32_t {
    Default = 0,
    Bullet,
    Physics,
    UI,
    Max
};

enum class RunPhase {
    Start, Update, Physics, Draw, Destroy
};

class TaskManager {
    // Type-erased task wrapper using concept-based dispatch
    struct ITask {
        virtual ~ITask() = default;
        virtual void start() = 0;
        virtual void update(float dt) = 0;
        virtual void physics(float dt) = 0;
        virtual void draw(RenderContext& ctx) = 0;
        virtual void release() = 0;
        virtual bool has_physics() const = 0;
        virtual bool has_draw() const = 0;

        // Threading introspection
        virtual ThreadingPolicy threading_policy() const = 0;
        virtual bool is_thread_aware() const = 0;
    };

    // TaskModel: concept-constrained bridge from concrete type to ITask
    // Uses if-constexpr to conditionally dispatch optional interfaces
    template<TaskLike T>
    struct TaskModel final : ITask {
        T task;
        template<typename... Args>
        explicit TaskModel(Args&&... args) : task(std::forward<Args>(args)...) {}

        void start() override { task.start(); }
        void update(float dt) override { task.update(dt); }

        void physics(float dt) override {
            if constexpr (requires(T& t, float d) { t.physics(d); })
                task.physics(dt);
        }

        void draw(RenderContext& ctx) override {
            if constexpr (Drawable<T, RenderContext>)
                task.draw(ctx);
        }

        void release() override { task.release(); }

        bool has_physics() const override {
            return requires(T& t, float d) { t.physics(d); };
        }

        bool has_draw() const override {
            return Drawable<T, RenderContext>;
        }

        ThreadingPolicy threading_policy() const override {
            if constexpr (ThreadAware<T>)
                return T::threading_policy();
            else
                return ThreadingPolicy::MainThread;
        }

        bool is_thread_aware() const override {
            return ThreadAware<T>;
        }
    };

    struct TaskEntry {
        uint64_t id;
        std::unique_ptr<ITask> impl;
        bool initialized = false;
        bool pending_destroy = false;
    };

    std::array<std::vector<TaskEntry>, static_cast<size_t>(TaskLayer::Max)> layers_;
    uint64_t next_id_ = 1;

public:
    // Concept-constrained registration: T must satisfy TaskLike
    template<TaskLike T, typename... Args>
    TaskHandle register_task(TaskLayer layer, Args&&... args) {
        uint64_t id = next_id_++;
        auto& vec = layers_[static_cast<size_t>(layer)];
        vec.push_back({id, std::make_unique<TaskModel<T>>(std::forward<Args>(args)...), false, false});
        return {id};
    }

    void destroy(TaskHandle handle);
    void run(RunPhase phase, float dt, RenderContext* ctx = nullptr);

    // Query
    size_t task_count() const;
    size_t task_count(TaskLayer layer) const;

    // Threading introspection: per-task info for visualization
    struct TaskThreadingInfo {
        uint64_t id;
        TaskLayer layer;
        ThreadingPolicy policy;
        bool thread_aware;  // true if type explicitly declares its policy
    };

    // Returns threading info for all active tasks
    std::vector<TaskThreadingInfo> threading_report() const;

    // Summary counts per policy
    struct ThreadingSummary {
        uint32_t main_thread = 0;
        uint32_t any_thread = 0;
        uint32_t parallel = 0;
        uint32_t total = 0;
    };
    ThreadingSummary threading_summary() const;
};
