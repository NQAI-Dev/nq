cmake_minimum_required(VERSION 3.16)
project(nq C)

# Tools (currently just nq-conv) live in their own subdirectory with
# their own CMakeLists. Adding here builds them as part of the top-level
# CMake project so `ctest` and friends can run them.
add_subdirectory(tools/nq-conv)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(NQ_BUILD_TESTS "Build nq_test target and enable ctest" ON)

# Threads needed for log_thread_safety on POSIX. On Windows FindThreads
# would be different, but for now this is fine.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# SDL3 — modern properties-based API, return-by-error instead of out-params.
find_package(SDL3 REQUIRED)

# nq core static library — the engine proper. Consumers link against this.
add_library(nq_core STATIC
    src/nq_graphics.c
    src/nq_input.c
    src/nq_log.c
    src/nq_texture.c
    src/nq_clock.c
    src/nq_tween.c
    src/nq_animation.c
    src/nq_atlas.c
    src/nq_node.c
    src/nq_scene.c
    src/nq_action.c
    src/nq_action_tween.c
    src/nq_action_delay.c
    src/nq_action_manager.c
    src/nq_input_sdl3.c
    src/nq_action_sequence.c
    src/nq_action_spawn.c
    src/nq_action_repeat.c
    src/nq_text.c
    src/nq_color.c
    src/nq_input_text.c
    src/nq_anim_color.c
    src/nq_action_tween_color.c
)
target_include_directories(nq_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(nq_core PUBLIC SDL3::SDL3 Threads::Threads)

# libm on Linux for powf/sinf/cosf in nq_tween.c. macOS bundles libm into
# libSystem (no extra link needed); Windows has math in the CRT.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(nq_core PUBLIC m)
endif()
target_compile_options(nq_core PRIVATE
    -Wall -Wextra
)

# First example: hello window with a bouncing colored square.
add_executable(nq_hello_window examples/hello_window.c)
target_link_libraries(nq_hello_window PRIVATE nq_core)

# Second example: animated_square — kitchen-sink demo wiring together
# nq_clock + nq_scene + nq_action_manager + nq_action_tween + nq_animation
# + nq_renderer. RGB channels cycle through sine-in-out independently so
# the box's colour is always smoothly shifting.
add_executable(nq_animated_square examples/animated_square.c)
target_link_libraries(nq_animated_square PRIVATE nq_core)

# Third example: controlled_square — arrow keys / WASD steer a box with
# friction and wall bouncing. Exercises the full input pipeline:
# SDL_PollEvent → nq_input_pump_sdl3_event → nq_input_* queries.
add_executable(nq_controlled_square examples/controlled_square.c)
target_link_libraries(nq_controlled_square PRIVATE nq_core)

# Fourth example: parallax — three coloured rectangles scroll at
# different speeds based on input. Uses nq_node + nq_scene (tree draw
# order) + nq_input + nq_clock. No textures — pure procedural rectangles
# so CI doesn't need any fixture files in tree.
add_executable(nq_parallax examples/parallax.c)
target_link_libraries(nq_parallax PRIVATE nq_core)

# Fifth example: animatedsprite — single sprite with 4 procedurally-
# generated frames in an NqAtlas, cycled via NqAnimFloat. Demonstrates
# nq_texture + nq_atlas + nq_anim_float pipeline end-to-end without
# needing real asset files in tree.
add_executable(nq_animatedsprite examples/animatedsprite.c)
target_link_libraries(nq_animatedsprite PRIVATE nq_core)

# SDL3's headers trip -Wpedantic at -std=c11 (GNU extensions upstream).
# Drop it only for SDL3 transitive headers; keep our own code strict.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(nq_hello_window PRIVATE -Wno-pedantic)
    target_compile_options(nq_core PRIVATE -Wno-pedantic)
endif()

# --- Tests ---
if(NQ_BUILD_TESTS)
    enable_testing()

    file(GLOB NQ_TEST_SOURCES CONFIGURE_DEPENDS "tests/test_*.c")

    add_executable(nq_test ${NQ_TEST_SOURCES})
    target_include_directories(nq_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(nq_test PRIVATE nq_core Threads::Threads SDL3::SDL3)

    add_test(NAME nq_test COMMAND nq_test)
endif()