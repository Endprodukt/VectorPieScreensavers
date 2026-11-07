# Constellations: a slow tour of the real night sky
# (README.md). Built from the top folder's CMakeLists.txt, which brings the SDK.
project(constellations VERSION 0.1.0 LANGUAGES C)

# The executable's name is the one package/game.ini names (executable = ...).
add_executable(saver_constellations src/main.c)
vp_program(saver_constellations)

add_custom_command(TARGET saver_constellations POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/package/game.ini
            $<TARGET_FILE_DIR:saver_constellations>/game.ini)
