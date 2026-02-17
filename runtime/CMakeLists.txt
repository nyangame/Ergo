add_executable(ergo_runtime
    main.cpp
    engine_context.cpp
    dll_loader.cpp
    plugin_loader.cpp
)

target_link_libraries(ergo_runtime PRIVATE ergo_system)
target_include_directories(ergo_runtime PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/game_interface
)

# Link dl on Linux for dynamic library loading
if(UNIX AND NOT APPLE)
    target_link_libraries(ergo_runtime PRIVATE dl)
endif()
