cmake_minimum_required(VERSION 3.20)
project(Ergo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(ERGO_BUILD_SAMPLES "Build sample games" ON)

# Engine Layer (static library)
add_subdirectory(engine)

# System Layer (static library)
add_subdirectory(system)

# Runtime (executable)
add_subdirectory(runtime)

# Sample Game (shared library)
if(ERGO_BUILD_SAMPLES)
    add_subdirectory(samples/shooting)
endif()
