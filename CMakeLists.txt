cmake_minimum_required(VERSION 3.11)
project(test_cs64_ini)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories( "${CMAKE_SOURCE_DIR}/src/" )

enable_testing()

add_executable(simple_test src-tests/simple.c)
add_test(NAME simple_test COMMAND $<TARGET_FILE:simple_test>)
