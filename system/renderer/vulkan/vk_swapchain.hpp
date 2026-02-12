#pragma once
#include <cstdint>

// Vulkan swapchain management
class VkSwapchainManager {
    struct Impl;
    Impl* impl_ = nullptr;

public:
    VkSwapchainManager() = default;
    ~VkSwapchainManager();

    bool create(void* surface_handle, uint32_t width, uint32_t height);
    bool acquire_next_image();
    bool present();
    void destroy();

    uint32_t width() const;
    uint32_t height() const;
};
