find_package(Vulkan REQUIRED)

add_library(ergo_system STATIC
    renderer/vulkan/vk_renderer.cpp
    renderer/vulkan/vk_pipeline.cpp
    renderer/vulkan/vk_swapchain.cpp
    input/desktop_input.cpp
    input/touch_input.cpp
    window/desktop_window.cpp
)

target_link_libraries(ergo_system PUBLIC Vulkan::Vulkan ergo_engine)
target_include_directories(ergo_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
