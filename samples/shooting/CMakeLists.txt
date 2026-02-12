add_library(shooting_game SHARED
    game_main.cpp
    player.cpp
    enemy.cpp
    bullet.cpp
    scenes/title_scene.cpp
    scenes/ingame_scene.cpp
)

target_link_libraries(shooting_game PRIVATE ergo_engine)
target_include_directories(shooting_game PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/game_interface
)
