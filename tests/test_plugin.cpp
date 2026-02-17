#include "framework/test_framework.hpp"
#include "game_interface/plugin_interface.h"
#include "runtime/plugin_loader.hpp"

using namespace ergo::test;

// ============================================================
// Fake plugin (statically linked for unit testing)
// ============================================================

namespace {

static bool g_fake_init_called = false;
static bool g_fake_update_called = false;
static bool g_fake_draw_called = false;
static bool g_fake_shutdown_called = false;
static float g_fake_last_dt = 0.0f;

void fake_on_init(const ErgoEngineAPI*) { g_fake_init_called = true; }
void fake_on_update(float dt) { g_fake_update_called = true; g_fake_last_dt = dt; }
void fake_on_draw() { g_fake_draw_called = true; }
void fake_on_shutdown() { g_fake_shutdown_called = true; }

void reset_fake() {
    g_fake_init_called = false;
    g_fake_update_called = false;
    g_fake_draw_called = false;
    g_fake_shutdown_called = false;
    g_fake_last_dt = 0.0f;
}

} // namespace

// ============================================================
// Test suites
// ============================================================

static TestSuite suite_plugin_interface("Plugin/Interface");
static TestSuite suite_plugin_manager("Plugin/Manager");

// ---------------------------------------------------------------------------
// Interface struct tests
// ---------------------------------------------------------------------------
static void register_interface_tests() {
    suite_plugin_interface.add("plugin_info_fields", [](TestContext& ctx) {
        ErgoPluginInfo info{};
        info.name        = "TestPlugin";
        info.version     = "1.0.0";
        info.description = "A test plugin";
        info.author      = "Ergo Team";

        ERGO_TEST_ASSERT_EQ(ctx, std::string(info.name), std::string("TestPlugin"));
        ERGO_TEST_ASSERT_EQ(ctx, std::string(info.version), std::string("1.0.0"));
        ERGO_TEST_ASSERT_EQ(ctx, std::string(info.description), std::string("A test plugin"));
        ERGO_TEST_ASSERT_EQ(ctx, std::string(info.author), std::string("Ergo Team"));
    });

    suite_plugin_interface.add("plugin_callbacks_lifecycle", [](TestContext& ctx) {
        reset_fake();

        ErgoPluginCallbacks callbacks{};
        callbacks.on_init     = fake_on_init;
        callbacks.on_update   = fake_on_update;
        callbacks.on_draw     = fake_on_draw;
        callbacks.on_shutdown = fake_on_shutdown;

        ERGO_TEST_ASSERT_FALSE(ctx, g_fake_init_called);
        callbacks.on_init(nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, g_fake_init_called);

        ERGO_TEST_ASSERT_FALSE(ctx, g_fake_update_called);
        callbacks.on_update(0.016f);
        ERGO_TEST_ASSERT_TRUE(ctx, g_fake_update_called);
        ERGO_TEST_ASSERT_NEAR(ctx, g_fake_last_dt, 0.016f, 0.0001f);

        ERGO_TEST_ASSERT_FALSE(ctx, g_fake_draw_called);
        callbacks.on_draw();
        ERGO_TEST_ASSERT_TRUE(ctx, g_fake_draw_called);

        ERGO_TEST_ASSERT_FALSE(ctx, g_fake_shutdown_called);
        callbacks.on_shutdown();
        ERGO_TEST_ASSERT_TRUE(ctx, g_fake_shutdown_called);
    });

    suite_plugin_interface.add("null_callbacks_are_safe", [](TestContext& ctx) {
        ErgoPluginCallbacks callbacks{};
        // All function pointers should be null by default (zero-init)
        ERGO_TEST_ASSERT(ctx, callbacks.on_init == nullptr);
        ERGO_TEST_ASSERT(ctx, callbacks.on_update == nullptr);
        ERGO_TEST_ASSERT(ctx, callbacks.on_draw == nullptr);
        ERGO_TEST_ASSERT(ctx, callbacks.on_shutdown == nullptr);
    });
}

// ---------------------------------------------------------------------------
// PluginManager tests (without actual DLL loading)
// ---------------------------------------------------------------------------
static void register_manager_tests() {
    suite_plugin_manager.add("initial_state_empty", [](TestContext& ctx) {
        PluginManager mgr;
        ERGO_TEST_ASSERT_EQ(ctx, mgr.count(), 0u);
        ERGO_TEST_ASSERT(ctx, mgr.get(1) == nullptr);
        ERGO_TEST_ASSERT(ctx, mgr.get(0) == nullptr);
    });

    suite_plugin_manager.add("load_nonexistent_dll_returns_zero", [](TestContext& ctx) {
        PluginManager mgr;
        uint64_t id = mgr.load("nonexistent_plugin.so");
        ERGO_TEST_ASSERT_EQ(ctx, id, uint64_t(0));
        ERGO_TEST_ASSERT_EQ(ctx, mgr.count(), 0u);
    });

    suite_plugin_manager.add("unload_invalid_id_returns_false", [](TestContext& ctx) {
        PluginManager mgr;
        ERGO_TEST_ASSERT_FALSE(ctx, mgr.unload(999));
    });

    suite_plugin_manager.add("unload_all_on_empty_is_safe", [](TestContext& ctx) {
        PluginManager mgr;
        mgr.unload_all();  // Should not crash
        ERGO_TEST_ASSERT_EQ(ctx, mgr.count(), 0u);
    });

    suite_plugin_manager.add("lifecycle_calls_on_empty_are_safe", [](TestContext& ctx) {
        PluginManager mgr;
        // These should not crash with no plugins loaded
        mgr.init_all(nullptr);
        mgr.update_all(0.016f);
        mgr.draw_all();
        mgr.shutdown_all();
        ERGO_TEST_ASSERT_EQ(ctx, mgr.count(), 0u);
    });

    suite_plugin_manager.add("plugin_dll_struct_validity", [](TestContext& ctx) {
        PluginDLL dll{};
        ERGO_TEST_ASSERT_FALSE(ctx, dll.valid());

        // Simulate a valid plugin (fake handle)
        dll.handle = reinterpret_cast<void*>(0xDEAD);
        ERGO_TEST_ASSERT_FALSE(ctx, dll.valid());  // still no info/callbacks

        ErgoPluginInfo info{"Test", "1.0", "desc", "author"};
        ErgoPluginCallbacks cbs{fake_on_init, fake_on_update, fake_on_draw, fake_on_shutdown};
        dll.info = &info;
        dll.callbacks = &cbs;
        ERGO_TEST_ASSERT_TRUE(ctx, dll.valid());
    });
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_plugin_tests(TestRunner& runner) {
    register_interface_tests();
    register_manager_tests();
    runner.add_suite(suite_plugin_interface);
    runner.add_suite(suite_plugin_manager);
}
