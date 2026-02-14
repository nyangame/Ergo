#include "demo_framework.hpp"
#include "engine/render/post_process.hpp"
#include <cstdio>

DEMO(PostProcess_Stack) {
    PostProcessStack stack;

    auto& fade = stack.add<FadeEffect>();
    fade.alpha = 0.5f;
    fade.r = 0; fade.g = 0; fade.b = 0;

    auto& vignette = stack.add<VignetteEffect>();
    vignette.intensity = 0.7f;

    auto& bloom = stack.add<BloomEffect>();
    bloom.threshold = 0.8f;
    bloom.intensity = 1.5f;
    bloom.blur_passes = 6;

    auto& color = stack.add<ColorGradeEffect>();
    color.brightness = 1.1f;
    color.contrast = 1.2f;
    color.saturation = 0.9f;

    std::printf("  Effects in stack: %zu\n", stack.effect_count());

    // List effects
    auto* f = dynamic_cast<FadeEffect*>(stack.get("Fade"));
    if (f) std::printf("    Fade: alpha=%.1f\n", f->alpha);

    auto* v = dynamic_cast<VignetteEffect*>(stack.get("Vignette"));
    if (v) std::printf("    Vignette: intensity=%.1f\n", v->intensity);

    auto* b = dynamic_cast<BloomEffect*>(stack.get("Bloom"));
    if (b) std::printf("    Bloom: threshold=%.1f passes=%d\n", b->threshold, b->blur_passes);

    auto* c = dynamic_cast<ColorGradeEffect*>(stack.get("ColorGrade"));
    if (c) std::printf("    ColorGrade: brightness=%.1f contrast=%.1f\n",
                       c->brightness, c->contrast);

    // Apply all
    stack.apply_all();
    std::printf("  Applied all effects\n");

    // Remove and verify
    stack.remove("Bloom");
    std::printf("  After removing Bloom: %zu effects\n", stack.effect_count());
}
