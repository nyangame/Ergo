#pragma once

// Vulkan pipeline management
// Handles shader compilation, pipeline layout, and pipeline creation
class VkPipelineManager {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    VkPipelineManager() = default;
    ~VkPipelineManager();

    bool create_sprite_pipeline();
    bool create_shape_pipeline();
    void destroy();
};
