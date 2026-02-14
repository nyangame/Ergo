#include "framework/test_framework.hpp"
#include "engine/animation/animation_clip.hpp"
#include "engine/animation/skeleton.hpp"
#include "engine/debug/profiler.hpp"
#include "engine/core/serialization.hpp"

using namespace ergo::test;

// ============================================================
// Animation/Clip suite
// ============================================================
static TestSuite animation_clip_suite("Animation/Clip");

static void register_animation_clip_tests() {

    animation_clip_suite.add("Skeleton_FindBone", [](TestContext& ctx) {
        Skeleton skel;
        Bone root;
        root.name = "root";
        root.parent_index = -1;
        skel.bones.push_back(root);

        Bone spine;
        spine.name = "spine";
        spine.parent_index = 0;
        skel.bones.push_back(spine);

        Bone head;
        head.name = "head";
        head.parent_index = 1;
        skel.bones.push_back(head);

        ERGO_TEST_ASSERT_EQ(ctx, skel.find_bone("root"), 0);
        ERGO_TEST_ASSERT_EQ(ctx, skel.find_bone("spine"), 1);
        ERGO_TEST_ASSERT_EQ(ctx, skel.find_bone("head"), 2);
        ERGO_TEST_ASSERT_EQ(ctx, skel.find_bone("missing"), -1);
    });

    animation_clip_suite.add("Skeleton_BoneCount", [](TestContext& ctx) {
        Skeleton skel;
        ERGO_TEST_ASSERT_EQ(ctx, skel.bone_count(), (size_t)0);

        Bone b; b.name = "root"; b.parent_index = -1;
        skel.bones.push_back(b);
        ERGO_TEST_ASSERT_EQ(ctx, skel.bone_count(), (size_t)1);
    });

    animation_clip_suite.add("AnimationClip_Properties", [](TestContext& ctx) {
        AnimationClip clip;
        clip.name = "idle";
        clip.duration = 2.0f;
        clip.loop = true;

        ERGO_TEST_ASSERT_TRUE(ctx, clip.name == "idle");
        ERGO_TEST_ASSERT_NEAR(ctx, clip.duration, 2.0f, 0.001f);
        ERGO_TEST_ASSERT_TRUE(ctx, clip.loop);
    });

    animation_clip_suite.add("BoneChannel_FindKeyframes_Single", [](TestContext& ctx) {
        BoneChannel ch;
        ch.bone_index = 0;
        ch.keyframes.push_back({0.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});

        auto [a, b] = ch.find_keyframes(0.0f);
        ERGO_TEST_ASSERT_EQ(ctx, a, (size_t)0);
        ERGO_TEST_ASSERT_EQ(ctx, b, (size_t)0);
    });

    animation_clip_suite.add("BoneChannel_FindKeyframes_Between", [](TestContext& ctx) {
        BoneChannel ch;
        ch.bone_index = 1;
        ch.keyframes.push_back({0.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});
        ch.keyframes.push_back({1.0f, {10, 0, 0}, Quat::identity(), {1, 1, 1}});

        auto [a, b] = ch.find_keyframes(0.5f);
        ERGO_TEST_ASSERT_EQ(ctx, a, (size_t)0);
        ERGO_TEST_ASSERT_EQ(ctx, b, (size_t)1);
    });

    animation_clip_suite.add("BoneChannel_FindKeyframes_BeyondEnd", [](TestContext& ctx) {
        BoneChannel ch;
        ch.bone_index = 2;
        ch.keyframes.push_back({0.0f, {0, 0, 0}, Quat::identity(), {1, 1, 1}});
        ch.keyframes.push_back({1.0f, {10, 0, 0}, Quat::identity(), {2, 2, 2}});

        auto [a, b] = ch.find_keyframes(2.0f);
        // Beyond end should return last keyframe
        ERGO_TEST_ASSERT_EQ(ctx, a, (size_t)1);
        ERGO_TEST_ASSERT_EQ(ctx, b, (size_t)1);
    });

    animation_clip_suite.add("BoneChannel_KeyframeData", [](TestContext& ctx) {
        BoneChannel ch;
        ch.bone_index = 0;
        ch.keyframes.push_back({0.5f, {1, 2, 3}, Quat::identity(), {1, 1, 1}});

        ERGO_TEST_ASSERT_NEAR(ctx, ch.keyframes[0].time, 0.5f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, ch.keyframes[0].position.x, 1.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, ch.keyframes[0].position.y, 2.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, ch.keyframes[0].position.z, 3.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, ch.keyframes[0].rotation.w, 1.0f, 0.001f);
    });
}

// ============================================================
// Debug/Profiler suite
// ============================================================
static TestSuite profiler_suite("Debug/Profiler");

static void register_profiler_tests() {

    profiler_suite.add("Profiler_BeginEnd", [](TestContext& ctx) {
        Profiler profiler;
        profiler.begin("test");
        // Do some trivial work
        volatile int x = 0;
        for (int i = 0; i < 1000; ++i) x += i;
        profiler.end();

        float ms = profiler.get("test");
        ERGO_TEST_ASSERT_TRUE(ctx, ms >= 0.0f);
    });

    profiler_suite.add("Profiler_Results", [](TestContext& ctx) {
        Profiler profiler;
        profiler.begin("section_a");
        profiler.end();
        profiler.begin("section_b");
        profiler.end();

        auto& results = profiler.results();
        ERGO_TEST_ASSERT_EQ(ctx, results.size(), (size_t)2);
        ERGO_TEST_ASSERT_TRUE(ctx, results.count("section_a") > 0);
        ERGO_TEST_ASSERT_TRUE(ctx, results.count("section_b") > 0);
    });

    profiler_suite.add("Profiler_Clear", [](TestContext& ctx) {
        Profiler profiler;
        profiler.begin("test");
        profiler.end();
        profiler.clear();
        ERGO_TEST_ASSERT_EQ(ctx, profiler.results().size(), (size_t)0);
    });

    profiler_suite.add("Profiler_GetNonexistent", [](TestContext& ctx) {
        Profiler profiler;
        float ms = profiler.get("nonexistent");
        ERGO_TEST_ASSERT_NEAR(ctx, ms, 0.0f, 0.001f);
    });

    profiler_suite.add("Profiler_Nested", [](TestContext& ctx) {
        Profiler profiler;
        profiler.begin("outer");
        profiler.begin("inner");
        profiler.end();
        profiler.end();

        ERGO_TEST_ASSERT_TRUE(ctx, profiler.get("outer") >= profiler.get("inner"));
    });
}

// ============================================================
// Core/Serialization suite
// ============================================================
static TestSuite serialization_suite("Core/Serialization");

static void register_serialization_tests() {
    using namespace ergo;

    serialization_suite.add("JsonValue_Null", [](TestContext& ctx) {
        JsonValue v;
        ERGO_TEST_ASSERT_TRUE(ctx, v.is_null());
    });

    serialization_suite.add("JsonValue_Bool", [](TestContext& ctx) {
        JsonValue v(true);
        ERGO_TEST_ASSERT_TRUE(ctx, v.is_bool());
        ERGO_TEST_ASSERT_TRUE(ctx, v.bool_val);
    });

    serialization_suite.add("JsonValue_Number", [](TestContext& ctx) {
        JsonValue v(42.0);
        ERGO_TEST_ASSERT_TRUE(ctx, v.is_number());
        ERGO_TEST_ASSERT_NEAR(ctx, v.number_val, 42.0, 0.001);
        ERGO_TEST_ASSERT_EQ(ctx, v.as_int(), 42);
    });

    serialization_suite.add("JsonValue_String", [](TestContext& ctx) {
        JsonValue v("hello");
        ERGO_TEST_ASSERT_TRUE(ctx, v.is_string());
        ERGO_TEST_ASSERT_TRUE(ctx, v.string_val == "hello");
    });

    serialization_suite.add("JsonValue_Array", [](TestContext& ctx) {
        JsonValue v(JsonArray{JsonValue(1), JsonValue(2), JsonValue(3)});
        ERGO_TEST_ASSERT_TRUE(ctx, v.is_array());
        ERGO_TEST_ASSERT_EQ(ctx, v.array_val.size(), (size_t)3);
        ERGO_TEST_ASSERT_NEAR(ctx, v[0].number_val, 1.0, 0.001);
        ERGO_TEST_ASSERT_NEAR(ctx, v[2].number_val, 3.0, 0.001);
    });

    serialization_suite.add("JsonValue_Object", [](TestContext& ctx) {
        JsonValue v(JsonObject{{"key", JsonValue("value")}});
        ERGO_TEST_ASSERT_TRUE(ctx, v.is_object());
        ERGO_TEST_ASSERT_TRUE(ctx, v["key"].is_string());
        ERGO_TEST_ASSERT_TRUE(ctx, v["key"].string_val == "value");
    });

    serialization_suite.add("Serialize_Vec2f", [](TestContext& ctx) {
        Vec2f original{3.14f, 2.71f};
        auto json = serialize(original);
        auto result = deserialize_vec2f(json);
        ERGO_TEST_ASSERT_NEAR(ctx, result.x, 3.14f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, result.y, 2.71f, 0.001f);
    });

    serialization_suite.add("Serialize_Vec3f", [](TestContext& ctx) {
        Vec3f original{1.0f, 2.0f, 3.0f};
        auto json = serialize(original);
        auto result = deserialize_vec3f(json);
        ERGO_TEST_ASSERT_NEAR(ctx, result.x, 1.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, result.y, 2.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, result.z, 3.0f, 0.001f);
    });

    serialization_suite.add("Serialize_Color", [](TestContext& ctx) {
        Color original{255, 128, 64, 200};
        auto json = serialize(original);
        auto result = deserialize_color(json);
        ERGO_TEST_ASSERT_EQ(ctx, result.r, (uint8_t)255);
        ERGO_TEST_ASSERT_EQ(ctx, result.g, (uint8_t)128);
        ERGO_TEST_ASSERT_EQ(ctx, result.b, (uint8_t)64);
        ERGO_TEST_ASSERT_EQ(ctx, result.a, (uint8_t)200);
    });

    serialization_suite.add("Serialize_Size2f", [](TestContext& ctx) {
        Size2f original{800.0f, 600.0f};
        auto json = serialize(original);
        auto result = deserialize_size2f(json);
        ERGO_TEST_ASSERT_NEAR(ctx, result.w, 800.0f, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, result.h, 600.0f, 0.001f);
    });

    serialization_suite.add("Serialize_Quat", [](TestContext& ctx) {
        Quat original = Quat::from_axis_angle(Vec3f::up(), 1.57f);
        auto json = serialize(original);
        auto result = deserialize_quat(json);
        ERGO_TEST_ASSERT_NEAR(ctx, result.x, original.x, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, result.y, original.y, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, result.z, original.z, 0.001f);
        ERGO_TEST_ASSERT_NEAR(ctx, result.w, original.w, 0.001f);
    });
}

// ============================================================
// Registration entry point
// ============================================================
void register_animation_debug_tests(TestRunner& runner) {
    register_animation_clip_tests();
    register_profiler_tests();
    register_serialization_tests();

    runner.add_suite(animation_clip_suite);
    runner.add_suite(profiler_suite);
    runner.add_suite(serialization_suite);
}
