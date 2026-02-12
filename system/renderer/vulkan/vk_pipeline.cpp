#include "vk_pipeline.hpp"

struct VkPipelineManager::Impl {
    // TODO: VkPipeline, VkPipelineLayout, VkShaderModule, etc.
};

VkPipelineManager::~VkPipelineManager() {
    destroy();
}

bool VkPipelineManager::create_sprite_pipeline() {
    // TODO: Create 2D sprite rendering pipeline
    return true;
}

bool VkPipelineManager::create_shape_pipeline() {
    // TODO: Create basic shape (rect/circle) rendering pipeline
    return true;
}

void VkPipelineManager::destroy() {
    delete impl_;
    impl_ = nullptr;
}
