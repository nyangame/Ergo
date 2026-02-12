cmake_minimum_required(VERSION 3.20)
project(ergo_editor_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ergo_editor_native SHARED
    editor_api.cpp
    editor_renderer.cpp
)

target_include_directories(ergo_editor_native PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(ergo_editor_native PRIVATE
    ergo_engine
    ergo_system
)

# Vulkan (propagated from ergo_system via ERGO_HAS_VULKAN define)
find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    target_link_libraries(ergo_editor_native PRIVATE Vulkan::Vulkan)
endif()

# Ensure symbols are exported
set_target_properties(ergo_editor_native PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
