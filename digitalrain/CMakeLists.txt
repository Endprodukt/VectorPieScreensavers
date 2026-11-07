# Digital Rain: green code raining down, with the cabinet's game names in it
# now and then (README.md). Built from the top folder's CMakeLists.txt,
# which brings the SDK.
project(digitalrain VERSION 0.2.0 LANGUAGES C)

# The executable's name is the one package/game.ini names (executable = ...).
add_executable(saver_digitalrain src/main.c)
vp_program(saver_digitalrain)

add_custom_command(TARGET saver_digitalrain POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/package/game.ini
            $<TARGET_FILE_DIR:saver_digitalrain>/game.ini)
