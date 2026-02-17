add_executable(ergo_demos
    main.cpp
    demo_math.cpp
    demo_task_system.cpp
    demo_state_machine.cpp
    demo_game_object.cpp
    demo_ecs.cpp
    demo_physics2d.cpp
    demo_physics3d.cpp
    demo_camera.cpp
    demo_easing.cpp
    demo_sprite_animation.cpp
    demo_input_map.cpp
    demo_time.cpp
    demo_serialization.cpp
    demo_scene_manager.cpp
    demo_particle.cpp
    demo_render_command.cpp
    demo_post_process.cpp
    demo_light.cpp
    demo_ui.cpp
    demo_shader.cpp
    demo_profiler.cpp
    demo_text.cpp
    demo_animation.cpp
    demo_particle_emitter_component.cpp
)

target_link_libraries(ergo_demos PRIVATE ergo_engine)
target_include_directories(ergo_demos PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/demos
)
