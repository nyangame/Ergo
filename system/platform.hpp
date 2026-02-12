#pragma once

// Compile-time platform selection
// Replaces CppSampleGame's DxLib dependency with Vulkan/WebGPU backends

#if defined(ERGO_PLATFORM_WEB)
    // Web version uses TypeScript implementation, not C++
    #error "Web platform uses TypeScript implementation, not C++"
#else
    // Native platforms all use Vulkan
    #include "renderer/vulkan/vk_renderer.hpp"
    #include "input/desktop_input.hpp"
    #include "window/desktop_window.hpp"

    namespace ergo {
        using PlatformRenderer = VulkanRenderer;
        using PlatformInput = DesktopInput;
        using PlatformWindow = DesktopWindow;
    }
#endif
