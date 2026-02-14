cmake_minimum_required(VERSION 3.20)
project(Ergo LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC-specific settings
if(MSVC)
    # Prevent windows.h from defining min/max macros
    add_compile_definitions(NOMINMAX)
    # Use standard-conforming preprocessor
    add_compile_options(/Zc:preprocessor)
endif()

# Options
option(ERGO_BUILD_SAMPLES "Build sample games" OFF)
option(ERGO_BUILD_EDITOR "Build editor native library" OFF)
option(ERGO_BUILD_TESTS "Build test assembly" ON)
option(ERGO_ENABLE_NETWORK "Enable network support" ON)
option(ERGO_FETCH_POCO "Download POCO via FetchContent (requires internet)" OFF)

# -------------------------------------------------------
# POCO C++ Libraries (optional network backend)
# -------------------------------------------------------
# Resolution order:
#   1. find_package(Poco) - system / vcpkg / conan install
#   2. FetchContent from GitHub (only if ERGO_FETCH_POCO=ON)
#   3. Fall back to POSIX / BSD socket backend
# -------------------------------------------------------
set(ERGO_HAS_POCO OFF)

if(ERGO_ENABLE_NETWORK)
    # 1) Try system-installed POCO
    find_package(Poco COMPONENTS Foundation Net JSON QUIET)
    if(Poco_FOUND)
        set(ERGO_HAS_POCO ON)
        message(STATUS "[Ergo] Found system POCO - using Poco::Net backend")
    elseif(ERGO_FETCH_POCO)
        # 2) Download and build POCO from source
        include(FetchContent)
        FetchContent_Declare(
            poco
            GIT_REPOSITORY https://github.com/pocoproject/poco.git
            GIT_TAG        poco-1.13.3-release
            GIT_SHALLOW    TRUE
        )
        set(ENABLE_NETSSL       OFF CACHE BOOL "" FORCE)
        set(ENABLE_CRYPTO       OFF CACHE BOOL "" FORCE)
        set(ENABLE_DATA         OFF CACHE BOOL "" FORCE)
        set(ENABLE_DATA_SQLITE  OFF CACHE BOOL "" FORCE)
        set(ENABLE_DATA_MYSQL   OFF CACHE BOOL "" FORCE)
        set(ENABLE_DATA_ODBC    OFF CACHE BOOL "" FORCE)
        set(ENABLE_DATA_POSTGRESQL OFF CACHE BOOL "" FORCE)
        set(ENABLE_MONGODB      OFF CACHE BOOL "" FORCE)
        set(ENABLE_REDIS        OFF CACHE BOOL "" FORCE)
        set(ENABLE_PDF          OFF CACHE BOOL "" FORCE)
        set(ENABLE_ZIP          OFF CACHE BOOL "" FORCE)
        set(ENABLE_PAGECOMPILER OFF CACHE BOOL "" FORCE)
        set(ENABLE_PAGECOMPILER_FILE2PAGE OFF CACHE BOOL "" FORCE)
        set(ENABLE_ACTIVERECORD OFF CACHE BOOL "" FORCE)
        set(ENABLE_ACTIVERECORD_COMPILER OFF CACHE BOOL "" FORCE)
        set(ENABLE_ENCODINGS    OFF CACHE BOOL "" FORCE)
        set(ENABLE_ENCODINGS_COMPILER OFF CACHE BOOL "" FORCE)
        set(ENABLE_XML          OFF CACHE BOOL "" FORCE)
        set(ENABLE_JWT          OFF CACHE BOOL "" FORCE)
        set(ENABLE_PROMETHEUS   OFF CACHE BOOL "" FORCE)
        set(ENABLE_TESTS        OFF CACHE BOOL "" FORCE)
        set(ENABLE_SAMPLES      OFF CACHE BOOL "" FORCE)
        set(ENABLE_FOUNDATION   ON  CACHE BOOL "" FORCE)
        set(ENABLE_JSON         ON  CACHE BOOL "" FORCE)
        set(ENABLE_NET          ON  CACHE BOOL "" FORCE)
        set(ENABLE_UTIL         OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(poco)
        set(ERGO_HAS_POCO ON)
        message(STATUS "[Ergo] Built POCO from source - using Poco::Net backend")
    else()
        message(STATUS "[Ergo] POCO not found - using POSIX socket backend")
        message(STATUS "[Ergo]   To use POCO: install libpoco-dev, or set -DERGO_FETCH_POCO=ON")
    endif()
endif()

# -------------------------------------------------------
# System Assembly: engine + system layers (static libs)
# -------------------------------------------------------

# Engine Layer (core, math, physics, render pipeline, resources, network)
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
# Test Assembly: GUI-independent test executable
# -------------------------------------------------------
if(ERGO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
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
