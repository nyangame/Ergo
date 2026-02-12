#pragma once
#include <cstdint>
#include "engine/math/vec2.hpp"
#include "engine/math/size2.hpp"
#include "engine/math/color.hpp"
#include "engine/resource/texture_handle.hpp"

// RenderContext: interface for game-side drawing
// Replaces CppSampleGame's DrawBox, DrawCircle, DrawString, etc.
struct RenderContext {
    virtual ~RenderContext() = default;
    virtual void draw_rect(Vec2f pos, Size2f size, Color color, bool filled) = 0;
    virtual void draw_circle(Vec2f center, float radius, Color color, bool filled) = 0;
    virtual void draw_sprite(Vec2f pos, Size2f size, TextureHandle tex, Rect uv) = 0;
    virtual void draw_text(Vec2f pos, const char* text, Color color, float scale) = 0;
};

class VulkanRenderer {
    // Vulkan internal state (VkInstance, VkDevice, etc.)
    struct Impl;
    Impl* impl_ = nullptr;

public:
    VulkanRenderer() = default;
    ~VulkanRenderer();

    // Satisfies RendererBackend concept
    bool initialize();
    void begin_frame();
    void end_frame();
    void shutdown();

    // Get RenderContext
    RenderContext* context();

    // Resource management
    TextureHandle load_texture(const char* path);
    void unload_texture(TextureHandle handle);
};
