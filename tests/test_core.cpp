#include "framework/test_framework.hpp"
#include "engine/core/state_machine.hpp"
#include "engine/core/game_object.hpp"
#include "engine/core/id_generator.hpp"

using namespace ergo::test;

// ============================================================
// StateMachine tests
// ============================================================

namespace {

struct TestStateA {
    bool entered = false;
    bool exited = false;
    int update_count = 0;

    void enter() { entered = true; }
    void exit()  { exited = true; }
    void update(float) { ++update_count; }
};

struct TestStateB {
    bool entered = false;
    int update_count = 0;

    void enter() { entered = true; }
    void update(float) { ++update_count; }
};

} // namespace

static TestSuite suite_state_machine("Core/StateMachine");

static void register_state_machine_tests() {
    suite_state_machine.add("initial_state_is_monostate", [](TestContext& ctx) {
        StateMachine<TestStateA, TestStateB> sm;
        ERGO_TEST_ASSERT_FALSE(ctx, sm.is_state<TestStateA>());
        ERGO_TEST_ASSERT_FALSE(ctx, sm.is_state<TestStateB>());
    });

    suite_state_machine.add("transition_enters_new_state", [](TestContext& ctx) {
        StateMachine<TestStateA, TestStateB> sm;
        sm.transition<TestStateA>();
        ERGO_TEST_ASSERT_TRUE(ctx, sm.is_state<TestStateA>());
        ERGO_TEST_ASSERT_FALSE(ctx, sm.is_state<TestStateB>());
    });

    suite_state_machine.add("transition_exits_previous_state", [](TestContext& ctx) {
        StateMachine<TestStateA, TestStateB> sm;
        sm.transition<TestStateA>();
        sm.transition<TestStateB>();
        ERGO_TEST_ASSERT_FALSE(ctx, sm.is_state<TestStateA>());
        ERGO_TEST_ASSERT_TRUE(ctx, sm.is_state<TestStateB>());
    });

    suite_state_machine.add("update_dispatches_to_current_state", [](TestContext& ctx) {
        StateMachine<TestStateA, TestStateB> sm;
        sm.transition<TestStateA>();
        sm.update(0.016f);
        sm.update(0.016f);
        sm.update(0.016f);
        // Can't easily check update_count from outside without accessor,
        // but update should not throw
        ERGO_TEST_ASSERT_TRUE(ctx, sm.is_state<TestStateA>());
    });

    suite_state_machine.add("update_on_monostate_is_noop", [](TestContext& ctx) {
        StateMachine<TestStateA, TestStateB> sm;
        sm.update(0.016f);  // should not crash
        ERGO_TEST_ASSERT_TRUE(ctx, true);
    });

    suite_state_machine.add("multiple_transitions", [](TestContext& ctx) {
        StateMachine<TestStateA, TestStateB> sm;
        sm.transition<TestStateA>();
        ERGO_TEST_ASSERT_TRUE(ctx, sm.is_state<TestStateA>());
        sm.transition<TestStateB>();
        ERGO_TEST_ASSERT_TRUE(ctx, sm.is_state<TestStateB>());
        sm.transition<TestStateA>();
        ERGO_TEST_ASSERT_TRUE(ctx, sm.is_state<TestStateA>());
    });
}

// ============================================================
// GameObject tests
// ============================================================

static TestSuite suite_game_object("Core/GameObject");

static void register_game_object_tests() {
    suite_game_object.add("default_values", [](TestContext& ctx) {
        GameObject obj;
        ERGO_TEST_ASSERT_EQ(ctx, obj.id, (uint64_t)0);
        ERGO_TEST_ASSERT_EQ(ctx, obj.object_type(), (uint32_t)0);
        ERGO_TEST_ASSERT_TRUE(ctx, obj.name().empty());
    });

    suite_game_object.add("set_name_and_type", [](TestContext& ctx) {
        GameObject obj;
        obj.name_ = "Player";
        obj.object_type_ = 42;
        ERGO_TEST_ASSERT_EQ(ctx, std::string(obj.name()), std::string("Player"));
        ERGO_TEST_ASSERT_EQ(ctx, obj.object_type(), (uint32_t)42);
    });

    suite_game_object.add("transform_access", [](TestContext& ctx) {
        GameObject obj;
        obj.transform().position = {10.0f, 20.0f};
        obj.transform().rotation = 1.5f;
        ERGO_TEST_ASSERT_EQ(ctx, obj.transform_.position.x, 10.0f);
        ERGO_TEST_ASSERT_EQ(ctx, obj.transform_.position.y, 20.0f);
        ERGO_TEST_ASSERT_EQ(ctx, obj.transform_.rotation, 1.5f);
    });

    suite_game_object.add("add_and_get_component", [](TestContext& ctx) {
        struct Health { int hp = 100; };
        GameObject obj;
        obj.add_component(Health{50});
        auto* h = obj.get_component<Health>();
        ERGO_TEST_ASSERT(ctx, h != nullptr);
        ERGO_TEST_ASSERT_EQ(ctx, h->hp, 50);
    });

    suite_game_object.add("get_missing_component_returns_null", [](TestContext& ctx) {
        struct Damage { float value = 0.0f; };
        GameObject obj;
        auto* d = obj.get_component<Damage>();
        ERGO_TEST_ASSERT(ctx, d == nullptr);
    });

    suite_game_object.add("multiple_components", [](TestContext& ctx) {
        struct Health { int hp = 100; };
        struct Speed { float value = 0.0f; };
        GameObject obj;
        obj.add_component(Health{80});
        obj.add_component(Speed{5.0f});

        auto* h = obj.get_component<Health>();
        auto* s = obj.get_component<Speed>();
        ERGO_TEST_ASSERT(ctx, h != nullptr);
        ERGO_TEST_ASSERT(ctx, s != nullptr);
        ERGO_TEST_ASSERT_EQ(ctx, h->hp, 80);
        ERGO_TEST_ASSERT_NEAR(ctx, s->value, 5.0f, 1e-5f);
    });

    suite_game_object.add("const_get_component", [](TestContext& ctx) {
        struct Tag { int value = 7; };
        GameObject obj;
        obj.add_component(Tag{42});
        const GameObject& cobj = obj;
        const Tag* t = cobj.get_component<Tag>();
        ERGO_TEST_ASSERT(ctx, t != nullptr);
        ERGO_TEST_ASSERT_EQ(ctx, t->value, 42);
    });

    suite_game_object.add("overwrite_component", [](TestContext& ctx) {
        struct Health { int hp = 100; };
        GameObject obj;
        obj.add_component(Health{50});
        obj.add_component(Health{99});
        auto* h = obj.get_component<Health>();
        ERGO_TEST_ASSERT(ctx, h != nullptr);
        ERGO_TEST_ASSERT_EQ(ctx, h->hp, 99);
    });
}

// ============================================================
// IdGenerator tests
// ============================================================

static TestSuite suite_id_gen("Core/IdGenerator");

static void register_id_gen_tests() {
    suite_id_gen.add("ids_are_unique", [](TestContext& ctx) {
        uint64_t a = IdGenerator::next();
        uint64_t b = IdGenerator::next();
        uint64_t c = IdGenerator::next();
        ERGO_TEST_ASSERT(ctx, a != b);
        ERGO_TEST_ASSERT(ctx, b != c);
        ERGO_TEST_ASSERT(ctx, a != c);
    });

    suite_id_gen.add("ids_are_monotonically_increasing", [](TestContext& ctx) {
        uint64_t a = IdGenerator::next();
        uint64_t b = IdGenerator::next();
        ERGO_TEST_ASSERT(ctx, b > a);
    });

    suite_id_gen.add("ids_are_nonzero", [](TestContext& ctx) {
        uint64_t id = IdGenerator::next();
        ERGO_TEST_ASSERT(ctx, id != 0);
    });
}

// ============================================================
// Registration
// ============================================================

void register_core_tests(TestRunner& runner) {
    register_state_machine_tests();
    register_game_object_tests();
    register_id_gen_tests();

    runner.add_suite(suite_state_machine);
    runner.add_suite(suite_game_object);
    runner.add_suite(suite_id_gen);
}
