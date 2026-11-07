# Neon Drive: an endless 80s drive (README.md). Built from the top folder's
# CMakeLists.txt, which brings the SDK.
project(neondrive VERSION 0.7.1 LANGUAGES C)

# The executable's name is the one package/game.ini names (executable = ...).
add_executable(saver_neondrive
    src/main.c
    src/road.c
    src/scenery.c
    src/models.c
    src/props.c
    src/traffic.c
    src/flyers.c
    src/car.c)
target_include_directories(saver_neondrive PRIVATE src)
vp_program(saver_neondrive)

# A test, not the screensaver: hours of driving as fast as the computer
# goes, for crashes that take long to come (src/main.c, SAVER_SOAK).
add_executable(saver_soak
    src/main.c
    src/road.c
    src/scenery.c
    src/models.c
    src/props.c
    src/traffic.c
    src/flyers.c
    src/car.c)
target_include_directories(saver_soak PRIVATE src)
target_compile_definitions(saver_soak PRIVATE SAVER_SOAK)
vp_program(saver_soak)

add_custom_command(TARGET saver_neondrive POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/package/game.ini
            $<TARGET_FILE_DIR:saver_neondrive>/game.ini)
