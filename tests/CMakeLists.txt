add_executable(ergo_tests
    test_main.cpp
    test_vec2.cpp
    test_vec3.cpp
    test_mat4.cpp
    test_quat.cpp
    test_hit_test.cpp
    test_task_system.cpp
    test_physics_system.cpp
    test_ecs.cpp
)

target_link_libraries(ergo_tests PRIVATE ergo_engine)
target_include_directories(ergo_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/tests
)

# Register with CTest
enable_testing()
add_test(NAME ergo_tests COMMAND ergo_tests)
