# Clock: the time on many faces, with the date and the weather
# (README.md). Built from the top folder's CMakeLists.txt, which brings the SDK.
project(clock VERSION 0.1.0 LANGUAGES C)

# The executable's name is the one package/game.ini names (executable = ...).
add_executable(saver_clock src/main.c src/weather.c)
vp_program(saver_clock)

add_custom_command(TARGET saver_clock POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/package/game.ini
            $<TARGET_FILE_DIR:saver_clock>/game.ini
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/package/clock.ini
            $<TARGET_FILE_DIR:saver_clock>/clock.ini)
