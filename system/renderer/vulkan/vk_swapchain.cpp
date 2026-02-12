#include "vk_swapchain.hpp"

struct VkSwapchainManager::Impl {
    uint32_t width = 0;
    uint32_t height = 0;
    // TODO: VkSwapchainKHR, VkSurfaceKHR, image views, framebuffers, etc.
};

VkSwapchainManager::~VkSwapchainManager() {
    destroy();
}

bool VkSwapchainManager::create(void* /*surface_handle*/, uint32_t width, uint32_t height) {
    if (impl_) destroy();
    impl_ = new Impl{};
    impl_->width = width;
    impl_->height = height;
    // TODO: Create Vulkan swapchain
    return true;
}

bool VkSwapchainManager::acquire_next_image() {
    if (!impl_) return false;
    // TODO: vkAcquireNextImageKHR
    return true;
}

bool VkSwapchainManager::present() {
    if (!impl_) return false;
    // TODO: vkQueuePresentKHR
    return true;
}

void VkSwapchainManager::destroy() {
    delete impl_;
    impl_ = nullptr;
}

uint32_t VkSwapchainManager::width() const {
    return impl_ ? impl_->width : 0;
}

uint32_t VkSwapchainManager::height() const {
    return impl_ ? impl_->height : 0;
}
