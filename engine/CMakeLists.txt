set(ERGO_ENGINE_SOURCES
    # Core
    core/task_system.cpp
    core/log.cpp
    core/scene_manager.cpp
    core/input_map.cpp
    core/serialization.cpp

    # Physics (2D)
    physics/hit_test.cpp
    physics/physics_system.cpp
    physics/raycast2d.cpp
    physics/spatial_grid.cpp

    # Physics (3D rigid body)
    physics/collision3d.cpp
    physics/rigid_body_world.cpp
    physics/cpu_physics.cpp
    physics/gpu_physics.cpp

    # Render pipeline
    render/render_pipeline.cpp
    render/particle_system.cpp

    # Resources
    resource/fbx_loader.cpp
    resource/image_loader.cpp
    resource/font_loader.cpp
    resource/resource_manager.cpp

    # Text rendering
    text/text_layout.cpp
    text/text_renderer.cpp
    text/rich_text.cpp

    # Shader composition
    shader/shader_library.cpp
    shader/shader_compiler.cpp
    shader/shader_optimizer.cpp

    # Animation
    animation/animation_player.cpp

    # UI
    ui/ui_manager.cpp
    ui/ui_widgets.cpp
    ui/ui_node.cpp
    ui/ui_canvas.cpp
    ui/ui_image_node.cpp
    ui/ui_hierarchy.cpp

    # Debug
    debug/debug_draw.cpp

    # Network (base sockets from core)
    net/socket.cpp

    # Behaviour
    core/behaviour/behaviour_tree.cpp

    # ECS
    ecs/world.cpp
)

# Network module (POCO-backed, optional)
if(ERGO_ENABLE_NETWORK)
    list(APPEND ERGO_ENGINE_SOURCES
        net/tcp_socket.cpp
        net/udp_socket.cpp
        net/http_client.cpp
        net/network_manager.cpp
    )
endif()

add_library(ergo_engine STATIC ${ERGO_ENGINE_SOURCES})

target_include_directories(ergo_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(ergo_engine PUBLIC cxx_std_20)

# Threading support (required for render pipeline + physics + network)
find_package(Threads REQUIRED)
target_link_libraries(ergo_engine PUBLIC Threads::Threads)

# Winsock2 (required by net/socket.cpp on Windows)
if(WIN32)
    target_link_libraries(ergo_engine PUBLIC ws2_32)
endif()

# Network configuration
if(ERGO_ENABLE_NETWORK)
    target_compile_definitions(ergo_engine PUBLIC ERGO_HAS_NETWORK=1)

    if(ERGO_HAS_POCO)
        # Link POCO libraries when available
        target_link_libraries(ergo_engine PUBLIC
            Poco::Foundation
            Poco::Net
            Poco::JSON
        )
        target_compile_definitions(ergo_engine PUBLIC ERGO_HAS_POCO=1)
    else()
        # Native socket fallback (POSIX / Winsock)
        target_compile_definitions(ergo_engine PUBLIC ERGO_HAS_POCO=0)
        if(WIN32)
            target_link_libraries(ergo_engine PUBLIC ws2_32)
        endif()
    endif()
else()
    target_compile_definitions(ergo_engine PUBLIC ERGO_HAS_NETWORK=0 ERGO_HAS_POCO=0)
endif()
