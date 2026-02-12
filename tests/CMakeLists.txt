# ============================================================
# Ergo Test Assembly
# Separate executable — runs independently from GUI / runtime.
# ============================================================

add_executable(ergo_tests
    main.cpp
    test_math.cpp
    test_physics.cpp
    test_core.cpp
)

target_include_directories(ergo_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(ergo_tests PRIVATE ergo_engine)
target_compile_features(ergo_tests PRIVATE cxx_std_20)
