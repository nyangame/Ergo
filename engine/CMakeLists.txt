add_library(ergo_engine STATIC
    # Core
    core/task_system.cpp

    # Physics (2D)
    physics/hit_test.cpp
    physics/physics_system.cpp

    # Physics (3D rigid body)
    physics/collision3d.cpp
    physics/rigid_body_world.cpp
    physics/cpu_physics.cpp
    physics/gpu_physics.cpp

    # Render pipeline
    render/render_pipeline.cpp

    # Resources
    resource/fbx_loader.cpp

    # Text rendering
    text/text_layout.cpp
    text/text_renderer.cpp
    text/rich_text.cpp

    # Shader composition
    shader/shader_library.cpp
    shader/shader_compiler.cpp
    shader/shader_optimizer.cpp
)

target_include_directories(ergo_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(ergo_engine PUBLIC cxx_std_20)

# Threading support (required for render pipeline + physics)
find_package(Threads REQUIRED)
target_link_libraries(ergo_engine PUBLIC Threads::Threads)
