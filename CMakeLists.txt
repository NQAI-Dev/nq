cmake_minimum_required(VERSION 3.16)
project(nq C)

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
    src/nq_input.c
    src/nq_log.c
    src/nq_input.c
    src/nq_log.c
    src/nq_input.c
)
target_include_directories(nq_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(nq_core PUBLIC SDL3::SDL3 Threads::Threads)
target_compile_options(nq_core PRIVATE
    -Wall -Wextra
)

# First example: hello window with a bouncing colored square.
add_executable(nq_hello_window examples/hello_window.c)
target_link_libraries(nq_hello_window PRIVATE nq_core)

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
    target_link_libraries(nq_test PRIVATE nq_core Threads::Threads)

    add_test(NAME nq_test COMMAND nq_test)
endif()
