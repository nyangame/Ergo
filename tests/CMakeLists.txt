add_executable(ergo_tests
    test_main.cpp

    # Math
    test_vec2.cpp
    test_vec3.cpp
    test_mat4.cpp
    test_quat.cpp
    test_color.cpp
    test_size2.cpp
    test_transform.cpp

    # Core
    test_task_system.cpp
    test_state_machine.cpp
    test_game_object.cpp
    test_camera.cpp
    test_easing.cpp
    test_sprite_animation.cpp
    test_input_map.cpp
    test_time.cpp
    test_id_generator.cpp
    test_serialization.cpp

    # Physics 2D
    test_hit_test.cpp
    test_physics_system.cpp
    test_spatial_grid.cpp

    # Physics 3D
    test_collision3d.cpp
    test_rigid_body.cpp

    # ECS
    test_ecs.cpp

    # Render
    test_render_command.cpp
    test_post_process.cpp
    test_light.cpp

    # UI
    test_ui.cpp

    # Shader
    test_shader_graph.cpp

    # Debug
    test_profiler.cpp

    # Animation
    test_animation_clip.cpp
)

target_link_libraries(ergo_tests PRIVATE ergo_engine)
target_include_directories(ergo_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/tests
)

# Register with CTest
add_test(NAME ergo_tests COMMAND ergo_tests)
