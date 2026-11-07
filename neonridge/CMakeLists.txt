# Neon Ridge: a still night by a neon plain under starlit mountains
# (README.md). Built from the top folder's CMakeLists.txt, which brings the SDK.
project(neonridge VERSION 0.1.0 LANGUAGES C)

# The executable's name is the one package/game.ini names (executable = ...).
add_executable(saver_neonridge src/main.c)
vp_program(saver_neonridge)

add_custom_command(TARGET saver_neonridge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/package/game.ini
            $<TARGET_FILE_DIR:saver_neonridge>/game.ini)
