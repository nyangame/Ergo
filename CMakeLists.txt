cmake_minimum_required(VERSION 3.20)
project(Ergo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(ERGO_BUILD_SAMPLES "Build sample games" OFF)
option(ERGO_BUILD_EDITOR "Build editor native library" OFF)

# -------------------------------------------------------
# System Assembly: engine + system layers (static libs)
# -------------------------------------------------------

# Engine Layer (core, math, physics, render pipeline, resources)
add_subdirectory(engine)

# System Layer (platform backends: Vulkan, input, window)
add_subdirectory(system)

# -------------------------------------------------------
# Application Assembly: runtime executable
# -------------------------------------------------------

# Runtime (main executable that loads game DLLs)
add_subdirectory(runtime)

# -------------------------------------------------------
# Editor Assembly: native shared library for MAUI editor
# -------------------------------------------------------
if(ERGO_BUILD_EDITOR)
    add_subdirectory(editor/Ergo.Editor.Native)
endif()

# -------------------------------------------------------
# Sample Games (purged from default build)
# -------------------------------------------------------
if(ERGO_BUILD_SAMPLES)
    # SampleGame (shooting) has been purged from the default build.
    # To build samples, set -DERGO_BUILD_SAMPLES=ON
    if(EXISTS "${CMAKE_SOURCE_DIR}/samples/shooting/CMakeLists.txt")
        add_subdirectory(samples/shooting)
    endif()
endif()
