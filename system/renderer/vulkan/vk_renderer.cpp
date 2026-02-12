#include "vk_renderer.hpp"

// Stub RenderContext implementation for initial skeleton
struct StubRenderContext final : RenderContext {
    void draw_rect(Vec2f, Size2f, Color, bool) override {}
    void draw_circle(Vec2f, float, Color, bool) override {}
    void draw_sprite(Vec2f, Size2f, TextureHandle, Rect) override {}
    void draw_text(Vec2f, const char*, Color, float) override {}
};

struct VulkanRenderer::Impl {
    StubRenderContext render_context;
    bool initialized = false;
};

VulkanRenderer::~VulkanRenderer() {
    shutdown();
}

bool VulkanRenderer::initialize() {
    if (impl_) return true;
    impl_ = new Impl{};
    impl_->initialized = true;
    // TODO: Initialize Vulkan (VkInstance, VkDevice, VkQueue, swapchain, etc.)
    return true;
}

void VulkanRenderer::begin_frame() {
    if (!impl_) return;
    // TODO: Acquire swapchain image, begin command buffer
}

void VulkanRenderer::end_frame() {
    if (!impl_) return;
    // TODO: End command buffer, submit, present
}

void VulkanRenderer::shutdown() {
    if (!impl_) return;
    // TODO: Destroy Vulkan resources
    delete impl_;
    impl_ = nullptr;
}

RenderContext* VulkanRenderer::context() {
    if (!impl_) return nullptr;
    return &impl_->render_context;
}

TextureHandle VulkanRenderer::load_texture(const char* /*path*/) {
    // TODO: Load texture via Vulkan
    return {0};
}

void VulkanRenderer::unload_texture(TextureHandle /*handle*/) {
    // TODO: Destroy Vulkan texture
}
