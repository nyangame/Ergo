#include "framework/test_framework.hpp"
#include "engine/ecs/world.hpp"
#include "engine/core/task_system.hpp"

using namespace ergo::test;

// ============================================================
// ECS tests
// ============================================================

namespace {

struct Position {
    float x = 0, y = 0;
};

struct Velocity {
    float dx = 0, dy = 0;
};

struct Health {
    int hp = 100;
};

struct SimpleTask {
    int start_count = 0;
    int update_count = 0;
    bool released = false;

    void start() { ++start_count; }
    void update(float /*dt*/) { ++update_count; }
    void release() { released = true; }
};

} // namespace

static TestSuite suite_ecs("ECS/World");

static void register_ecs_tests() {
    suite_ecs.add("create_entity", [](TestContext& ctx) {
        World world;
        uint64_t e1 = world.create_entity();
        uint64_t e2 = world.create_entity();
        ERGO_TEST_ASSERT(ctx, e1 != e2);
        ERGO_TEST_ASSERT_TRUE(ctx, world.entity_exists(e1));
        ERGO_TEST_ASSERT_TRUE(ctx, world.entity_exists(e2));
        ERGO_TEST_ASSERT_EQ(ctx, world.entity_count(), 2u);
    });

    suite_ecs.add("destroy_entity", [](TestContext& ctx) {
        World world;
        uint64_t e = world.create_entity();
        world.destroy_entity(e);
        ERGO_TEST_ASSERT_FALSE(ctx, world.entity_exists(e));
        ERGO_TEST_ASSERT_EQ(ctx, world.entity_count(), 0u);
    });

    suite_ecs.add("add_and_get_component", [](TestContext& ctx) {
        World world;
        uint64_t e = world.create_entity();
        world.add_component(e, Position{10.0f, 20.0f});

        auto* pos = world.get_component<Position>(e);
        ERGO_TEST_ASSERT(ctx, pos != nullptr);
        ERGO_TEST_ASSERT_NEAR(ctx, pos->x, 10.0f, 1e-6f);
        ERGO_TEST_ASSERT_NEAR(ctx, pos->y, 20.0f, 1e-6f);
    });

    suite_ecs.add("has_component", [](TestContext& ctx) {
        World world;
        uint64_t e = world.create_entity();
        ERGO_TEST_ASSERT_FALSE(ctx, world.has_component<Position>(e));

        world.add_component(e, Position{});
        ERGO_TEST_ASSERT_TRUE(ctx, world.has_component<Position>(e));
        ERGO_TEST_ASSERT_FALSE(ctx, world.has_component<Velocity>(e));
    });

    suite_ecs.add("query_entities", [](TestContext& ctx) {
        World world;
        uint64_t e1 = world.create_entity();
        world.add_component(e1, Position{1.0f, 0.0f});
        world.add_component(e1, Velocity{2.0f, 0.0f});

        uint64_t e2 = world.create_entity();
        world.add_component(e2, Position{3.0f, 0.0f});
        // e2 has no Velocity

        int count = 0;
        world.each<Position, Velocity>([&](uint64_t id, Position& pos, Velocity& vel) {
            (void)id;
            pos.x += vel.dx;
            ++count;
        });

        ERGO_TEST_ASSERT_EQ(ctx, count, 1);

        auto* pos = world.get_component<Position>(e1);
        ERGO_TEST_ASSERT(ctx, pos != nullptr);
        ERGO_TEST_ASSERT_NEAR(ctx, pos->x, 3.0f, 1e-6f);  // 1.0 + 2.0
    });
}

// ============================================================
// TaskSystem tests
// ============================================================

static TestSuite suite_task("Core/TaskSystem");

static void register_task_tests() {
    suite_task.add("register_and_count", [](TestContext& ctx) {
        TaskManager mgr;
        mgr.register_task<SimpleTask>(TaskLayer::Default);
        mgr.register_task<SimpleTask>(TaskLayer::Default);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.task_count(), 2u);
    });

    suite_task.add("update_phase", [](TestContext& ctx) {
        TaskManager mgr;
        auto handle = mgr.register_task<SimpleTask>(TaskLayer::Default);
        (void)handle;
        mgr.run(RunPhase::Update, 0.016f);
        ERGO_TEST_ASSERT_TRUE(ctx, mgr.task_count() > 0);
    });

    suite_task.add("destroy", [](TestContext& ctx) {
        TaskManager mgr;
        auto h1 = mgr.register_task<SimpleTask>(TaskLayer::Default);
        auto h2 = mgr.register_task<SimpleTask>(TaskLayer::Default);
        (void)h2;
        ERGO_TEST_ASSERT_EQ(ctx, mgr.task_count(), 2u);

        mgr.destroy(h1);
        mgr.run(RunPhase::Destroy, 0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.task_count(), 1u);
    });

    suite_task.add("layer_count", [](TestContext& ctx) {
        TaskManager mgr;
        mgr.register_task<SimpleTask>(TaskLayer::Default);
        mgr.register_task<SimpleTask>(TaskLayer::Bullet);
        mgr.register_task<SimpleTask>(TaskLayer::Bullet);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.task_count(TaskLayer::Default), 1u);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.task_count(TaskLayer::Bullet), 2u);
    });
}

// ============================================================
// Registration
// ============================================================

void register_ecs_task_tests(TestRunner& runner) {
    register_ecs_tests();
    register_task_tests();

    runner.add_suite(suite_ecs);
    runner.add_suite(suite_task);
}
