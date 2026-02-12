add_library(ergo_engine STATIC
    core/task_system.cpp
    physics/hit_test.cpp
    physics/physics_system.cpp
)

target_include_directories(ergo_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(ergo_engine PUBLIC cxx_std_20)
