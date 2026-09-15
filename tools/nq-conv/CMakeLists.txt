cmake_minimum_required(VERSION 3.16)
project(nq_conv C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(nq-conv main.c)
# SDL3-free by design — see header comment in main.c.
target_compile_options(nq-conv PRIVATE -Wall -Wextra)
