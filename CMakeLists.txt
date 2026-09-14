cmake_minimum_required(VERSION 3.16)
project(nq C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# SDL3 — modern properties-based API, return-by-error instead of out-params.
# Required because we're targeting the current SDL line (2026); SDL2 is
# legacy and the API differences are too wide to dual-target.
find_package(SDL3 REQUIRED)

# First example: a hello window with a render loop and ESC-to-quit.
# The engine proper will live under include/nq/ and src/ once the
# scaffolding settles; for now we start with one demonstrable binary.
add_executable(nq_hello_window examples/hello_window.c)
target_link_libraries(nq_hello_window PRIVATE SDL3::SDL3)

# Public headers under include/ are exposed to consumers of the engine
# library (once it exists); examples/ sees the same include path.
target_include_directories(nq_hello_window PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compile flags: warnings as errors keeps the engine honest. The C
# standard is C11 — anything newer is overkill for a 2D engine and
# would unnecessarily narrow portability to old toolchains.
target_compile_options(nq_hello_window PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)

# SDL3's headers warn on -Wpedantic at -std=c11 due to GNU extensions
# in upstream headers. Drop -Wpedantic only for SDL3 transitive headers,
# keep it strict for our own code.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(nq_hello_window PRIVATE
        -Wno-pedantic
    )
endif()
