#include "test_framework.hpp"
#include "engine/core/task_system.hpp"

namespace {

struct SimpleTask {
    int start_count = 0;
    int update_count = 0;
    bool released = false;

    void start() { ++start_count; }
    void update(float /*dt*/) { ++update_count; }
    void release() { released = true; }
};

struct DrawableTask {
    int draw_count = 0;
    void start() {}
    void update(float) {}
    void draw(RenderContext&) { ++draw_count; }
    void release() {}
};

} // anonymous namespace

TEST_CASE(task_system_register_and_count) {
    TaskManager mgr;
    mgr.register_task<SimpleTask>(TaskLayer::Default);
    mgr.register_task<SimpleTask>(TaskLayer::Default);
    ASSERT_EQ(mgr.task_count(), 2u);
}

TEST_CASE(task_system_update_phase) {
    TaskManager mgr;
    auto handle = mgr.register_task<SimpleTask>(TaskLayer::Default);
    mgr.run(RunPhase::Update, 0.016f);
    // First update triggers start + update
    ASSERT_TRUE(mgr.task_count() > 0);
}

TEST_CASE(task_system_destroy) {
    TaskManager mgr;
    auto h1 = mgr.register_task<SimpleTask>(TaskLayer::Default);
    auto h2 = mgr.register_task<SimpleTask>(TaskLayer::Default);
    ASSERT_EQ(mgr.task_count(), 2u);

    mgr.destroy(h1);
    mgr.run(RunPhase::Destroy, 0.0f);
    ASSERT_EQ(mgr.task_count(), 1u);
}

TEST_CASE(task_system_layer_count) {
    TaskManager mgr;
    mgr.register_task<SimpleTask>(TaskLayer::Default);
    mgr.register_task<SimpleTask>(TaskLayer::Bullet);
    mgr.register_task<SimpleTask>(TaskLayer::Bullet);
    ASSERT_EQ(mgr.task_count(TaskLayer::Default), 1u);
    ASSERT_EQ(mgr.task_count(TaskLayer::Bullet), 2u);
}
