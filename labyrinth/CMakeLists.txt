# Labyrinth: a walk through an endless line of mazes (README.md). Built from
# the top folder's CMakeLists.txt, which brings the SDK.
project(labyrinth VERSION 0.1.0 LANGUAGES C)

# The executable's name is the one package/game.ini names (executable = ...).
add_executable(saver_labyrinth src/main.c)
vp_program(saver_labyrinth)

add_custom_command(TARGET saver_labyrinth POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/package/game.ini
            $<TARGET_FILE_DIR:saver_labyrinth>/game.ini)
