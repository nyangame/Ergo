#include "framework/test_framework.hpp"
#include "engine/render/render_command.hpp"
#include "engine/render/command_buffer.hpp"
#include "engine/render/double_buffer.hpp"
#include "engine/render/post_process.hpp"
#include "engine/render/light.hpp"

using namespace ergo::test;

// ============================================================
// CommandBuffer tests
// ============================================================

static TestSuite suite_command_buffer("Render/CommandBuffer");

static void register_command_buffer_tests() {
    suite_command_buffer.add("CommandBuffer_PushAndSize", [](TestContext& ctx) {
        CommandBuffer buf;
        ERGO_TEST_ASSERT_TRUE(ctx, buf.empty());
        ERGO_TEST_ASSERT_EQ(ctx, buf.size(), (size_t)0);

        buf.push(RenderCmd_Clear{{0, 0, 0, 255}, 1.0f});
        ERGO_TEST_ASSERT_FALSE(ctx, buf.empty());
        ERGO_TEST_ASSERT_EQ(ctx, buf.size(), (size_t)1);
    });

    suite_command_buffer.add("CommandBuffer_Clear", [](TestContext& ctx) {
        CommandBuffer buf;
        buf.push(RenderCmd_Clear{});
        buf.push(RenderCmd_DrawRect{});
        buf.clear();
        ERGO_TEST_ASSERT_TRUE(ctx, buf.empty());
    });

    suite_command_buffer.add("CommandBuffer_Merge", [](TestContext& ctx) {
        CommandBuffer a, b;
        a.push(RenderCmd_Clear{});
        b.push(RenderCmd_DrawRect{});
        b.push(RenderCmd_DrawCircle{});

        a.merge(b);
        ERGO_TEST_ASSERT_EQ(ctx, a.size(), (size_t)3);
    });

    suite_command_buffer.add("DoubleBuffer_WriteRead", [](TestContext& ctx) {
        DoubleBufferedCommands db;

        db.write_buffer().push(RenderCmd_Clear{});
        ERGO_TEST_ASSERT_EQ(ctx, db.write_buffer().size(), (size_t)1);
        ERGO_TEST_ASSERT_EQ(ctx, db.read_buffer().size(), (size_t)0);
    });

    suite_command_buffer.add("DoubleBuffer_Swap", [](TestContext& ctx) {
        DoubleBufferedCommands db;

        db.write_buffer().push(RenderCmd_Clear{});
        db.write_buffer().push(RenderCmd_DrawRect{});
        db.swap();

        // After swap, read buffer should have old data, write buffer is clear
        ERGO_TEST_ASSERT_EQ(ctx, db.read_buffer().size(), (size_t)2);
        ERGO_TEST_ASSERT_EQ(ctx, db.write_buffer().size(), (size_t)0);
    });

    suite_command_buffer.add("SharedCommandCollector_SubmitAndTake", [](TestContext& ctx) {
        SharedCommandCollector collector;

        CommandBuffer buf1;
        buf1.push(RenderCmd_Clear{});
        CommandBuffer buf2;
        buf2.push(RenderCmd_DrawRect{});
        buf2.push(RenderCmd_DrawCircle{});

        collector.submit(buf1);
        collector.submit(buf2);

        auto merged = collector.take();
        ERGO_TEST_ASSERT_EQ(ctx, merged.size(), (size_t)3);
    });
}

// ============================================================
// PostProcess tests
// ============================================================

static TestSuite suite_post_process("Render/PostProcess");

static void register_post_process_tests() {
    suite_post_process.add("PostProcess_AddEffect", [](TestContext& ctx) {
        PostProcessStack stack;
        stack.add<FadeEffect>();
        ERGO_TEST_ASSERT_EQ(ctx, stack.effect_count(), (size_t)1);
    });

    suite_post_process.add("PostProcess_GetEffect", [](TestContext& ctx) {
        PostProcessStack stack;
        auto& fade = stack.add<FadeEffect>();
        fade.alpha = 0.7f;

        auto* found = stack.get("Fade");
        ERGO_TEST_ASSERT_TRUE(ctx, found != nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, found->name == "Fade");
    });

    suite_post_process.add("PostProcess_RemoveEffect", [](TestContext& ctx) {
        PostProcessStack stack;
        stack.add<FadeEffect>();
        stack.add<BloomEffect>();
        ERGO_TEST_ASSERT_EQ(ctx, stack.effect_count(), (size_t)2);

        stack.remove("Fade");
        ERGO_TEST_ASSERT_EQ(ctx, stack.effect_count(), (size_t)1);
        ERGO_TEST_ASSERT_TRUE(ctx, stack.get("Fade") == nullptr);
        ERGO_TEST_ASSERT_TRUE(ctx, stack.get("Bloom") != nullptr);
    });

    suite_post_process.add("PostProcess_Clear", [](TestContext& ctx) {
        PostProcessStack stack;
        stack.add<FadeEffect>();
        stack.add<VignetteEffect>();
        stack.add<BloomEffect>();
        stack.clear();
        ERGO_TEST_ASSERT_EQ(ctx, stack.effect_count(), (size_t)0);
    });

    suite_post_process.add("PostProcess_ApplyAll", [](TestContext& ctx) {
        PostProcessStack stack;
        auto& fade = stack.add<FadeEffect>();
        fade.alpha = 0.5f;
        auto& bloom = stack.add<BloomEffect>();
        bloom.threshold = 0.8f;
        // Should not crash
        stack.apply_all();
        ERGO_TEST_ASSERT_EQ(ctx, stack.effect_count(), (size_t)2);
    });
}

// ============================================================
// Light tests
// ============================================================

static TestSuite suite_light("Render/Light");

static void register_light_tests() {
    suite_light.add("LightManager_AddLight", [](TestContext& ctx) {
        LightManager mgr;
        Light l;
        l.type = LightType::Directional;
        l.intensity = 1.5f;
        size_t idx = mgr.add_light(l);
        ERGO_TEST_ASSERT_EQ(ctx, idx, (size_t)0);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.light_count(), (size_t)1);
    });

    suite_light.add("LightManager_GetLight", [](TestContext& ctx) {
        LightManager mgr;
        Light l;
        l.type = LightType::Point;
        l.intensity = 2.0f;
        l.range = 10.0f;
        mgr.add_light(l);

        auto* found = mgr.get_light(0);
        ERGO_TEST_ASSERT_TRUE(ctx, found != nullptr);
        ERGO_TEST_ASSERT_NEAR(ctx, found->intensity, 2.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, found->range, 10.0f, 0.001f);
    });

    suite_light.add("LightManager_RemoveLight", [](TestContext& ctx) {
        LightManager mgr;
        Light l1; l1.type = LightType::Directional;
        Light l2; l2.type = LightType::Point;
        mgr.add_light(l1);
        mgr.add_light(l2);

        mgr.remove_light(0);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.light_count(), (size_t)1);
    });

    suite_light.add("LightManager_MaxLights", [](TestContext& ctx) {
        LightManager mgr;
        for (size_t i = 0; i < LightManager::MAX_LIGHTS; ++i) {
            Light l;
            mgr.add_light(l);
        }
        ERGO_TEST_ASSERT_EQ(ctx, mgr.light_count(), LightManager::MAX_LIGHTS);

        // Adding beyond max should fail
        size_t idx = mgr.add_light(Light{});
        ERGO_TEST_ASSERT_EQ(ctx, idx, SIZE_MAX);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.light_count(), LightManager::MAX_LIGHTS);
    });

    suite_light.add("LightManager_Ambient", [](TestContext& ctx) {
        LightManager mgr;
        mgr.set_ambient({100, 120, 140, 255});
        ERGO_TEST_ASSERT_EQ(ctx, mgr.ambient().r, (uint8_t)100);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.ambient().g, (uint8_t)120);
        ERGO_TEST_ASSERT_EQ(ctx, mgr.ambient().b, (uint8_t)140);
    });

    suite_light.add("LightManager_Clear", [](TestContext& ctx) {
        LightManager mgr;
        mgr.add_light(Light{});
        mgr.add_light(Light{});
        mgr.clear();
        ERGO_TEST_ASSERT_EQ(ctx, mgr.light_count(), (size_t)0);
    });
}

// ============================================================
// Registration
// ============================================================

void register_render_tests(TestRunner& runner) {
    register_command_buffer_tests();
    register_post_process_tests();
    register_light_tests();

    runner.add_suite(suite_command_buffer);
    runner.add_suite(suite_post_process);
    runner.add_suite(suite_light);
}
