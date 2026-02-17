# ============================================================
# Ergo Test Assembly
# Separate executable — runs independently from GUI / runtime.
# ============================================================

add_executable(ergo_tests
    main.cpp
    test_math.cpp
    test_physics.cpp
    test_core.cpp
    test_gameplay.cpp
    test_physics_extended.cpp
    test_render.cpp
    test_ui_shader.cpp
    test_animation_debug.cpp
    test_ecs_task.cpp
    test_particle_emitter_component.cpp
)

target_link_libraries(ergo_tests PRIVATE ergo_engine)
target_include_directories(ergo_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/tests
)
target_compile_features(ergo_tests PRIVATE cxx_std_20)

# Register with CTest
add_test(NAME ergo_tests COMMAND ergo_tests)
