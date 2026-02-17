find_package(Vulkan QUIET)

add_library(ergo_system STATIC
    renderer/vulkan/vk_renderer.cpp
    renderer/vulkan/vk_pipeline.cpp
    renderer/vulkan/vk_swapchain.cpp
    input/desktop_input.cpp
    input/touch_input.cpp
    input/gamepad_input.cpp
    window/desktop_window.cpp
    audio/audio_engine.cpp
)

if(ERGO_BUILD_EDITOR)
    set_target_properties(ergo_system PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(ergo_system PUBLIC ergo_engine)
target_include_directories(ergo_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(Vulkan_FOUND)
    target_link_libraries(ergo_system PUBLIC Vulkan::Vulkan)
    target_compile_definitions(ergo_system PUBLIC ERGO_HAS_VULKAN=1)
else()
    message(STATUS "[Ergo] Vulkan not found - building with stub renderer")
    target_compile_definitions(ergo_system PUBLIC ERGO_HAS_VULKAN=0)
endif()
