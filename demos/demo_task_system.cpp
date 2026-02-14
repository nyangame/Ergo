#include "demo_framework.hpp"
#include "engine/core/task_system.hpp"
#include <cstdio>

namespace {

struct DemoTask {
    int id;
    bool started = false;
    int update_count = 0;

    void start() {
        started = true;
        std::printf("    Task %d: start()\n", id);
    }
    void update(float dt) {
        ++update_count;
        std::printf("    Task %d: update(dt=%.3f) count=%d\n", id, dt, update_count);
    }
    void release() {
        std::printf("    Task %d: release()\n", id);
    }
};

struct PhysicsTask {
    void start() { std::printf("    PhysicsTask: start()\n"); }
    void update(float dt) { std::printf("    PhysicsTask: update(dt=%.3f)\n", dt); }
    void physics(float dt) { std::printf("    PhysicsTask: physics(dt=%.3f)\n", dt); }
    void release() { std::printf("    PhysicsTask: release()\n"); }
};

} // namespace

DEMO(TaskSystem_Lifecycle) {
    TaskManager mgr;

    auto h1 = mgr.register_task<DemoTask>(TaskLayer::Default, 1);
    auto h2 = mgr.register_task<DemoTask>(TaskLayer::Bullet, 2);
    auto h3 = mgr.register_task<PhysicsTask>(TaskLayer::Physics);

    std::printf("  Registered %zu tasks\n", mgr.task_count());

    std::printf("  --- RunPhase::Start ---\n");
    mgr.run(RunPhase::Start, 0.0f);

    std::printf("  --- RunPhase::Update (dt=0.016) ---\n");
    mgr.run(RunPhase::Update, 0.016f);

    std::printf("  --- RunPhase::Physics (dt=0.016) ---\n");
    mgr.run(RunPhase::Physics, 0.016f);

    std::printf("  --- Destroying task 1 ---\n");
    mgr.destroy(h1);
    mgr.run(RunPhase::Destroy, 0.0f);
    std::printf("  Remaining tasks: %zu\n", mgr.task_count());
}

DEMO(TaskSystem_Layers) {
    TaskManager mgr;

    mgr.register_task<DemoTask>(TaskLayer::Default, 10);
    mgr.register_task<DemoTask>(TaskLayer::Default, 11);
    mgr.register_task<DemoTask>(TaskLayer::Bullet, 20);
    mgr.register_task<DemoTask>(TaskLayer::UI, 30);

    std::printf("  Default layer: %zu tasks\n", mgr.task_count(TaskLayer::Default));
    std::printf("  Bullet layer:  %zu tasks\n", mgr.task_count(TaskLayer::Bullet));
    std::printf("  Physics layer: %zu tasks\n", mgr.task_count(TaskLayer::Physics));
    std::printf("  UI layer:      %zu tasks\n", mgr.task_count(TaskLayer::UI));
    std::printf("  Total:         %zu tasks\n", mgr.task_count());
}
