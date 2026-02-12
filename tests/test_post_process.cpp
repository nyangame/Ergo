#include "test_framework.hpp"
#include "engine/render/post_process.hpp"

TEST_CASE(PostProcess_AddEffect) {
    PostProcessStack stack;
    stack.add<FadeEffect>();
    ASSERT_EQ(stack.effect_count(), (size_t)1);
}

TEST_CASE(PostProcess_GetEffect) {
    PostProcessStack stack;
    auto& fade = stack.add<FadeEffect>();
    fade.alpha = 0.7f;

    auto* found = stack.get("Fade");
    ASSERT_TRUE(found != nullptr);
    ASSERT_TRUE(found->name == "Fade");
}

TEST_CASE(PostProcess_RemoveEffect) {
    PostProcessStack stack;
    stack.add<FadeEffect>();
    stack.add<BloomEffect>();
    ASSERT_EQ(stack.effect_count(), (size_t)2);

    stack.remove("Fade");
    ASSERT_EQ(stack.effect_count(), (size_t)1);
    ASSERT_TRUE(stack.get("Fade") == nullptr);
    ASSERT_TRUE(stack.get("Bloom") != nullptr);
}

TEST_CASE(PostProcess_Clear) {
    PostProcessStack stack;
    stack.add<FadeEffect>();
    stack.add<VignetteEffect>();
    stack.add<BloomEffect>();
    stack.clear();
    ASSERT_EQ(stack.effect_count(), (size_t)0);
}

TEST_CASE(PostProcess_ApplyAll) {
    PostProcessStack stack;
    auto& fade = stack.add<FadeEffect>();
    fade.alpha = 0.5f;
    auto& bloom = stack.add<BloomEffect>();
    bloom.threshold = 0.8f;
    // Should not crash
    stack.apply_all();
    ASSERT_EQ(stack.effect_count(), (size_t)2);
}
