cmake_minimum_required(VERSION 3.11)
project(test_cs64_ini)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories( "${CMAKE_SOURCE_DIR}/src/" )

enable_testing()

add_executable(simple_test src-tests/simple.c)
add_test(NAME simple_test COMMAND $<TARGET_FILE:simple_test>)

add_executable(unicode_test src-tests/unicode.c)
add_test(NAME unicode_test COMMAND $<TARGET_FILE:unicode_test>)

add_executable(token_test src-tests/token.c)
add_test(NAME token_test COMMAND $<TARGET_FILE:token_test>)

add_executable(hash_table_test src-tests/hash_table.c)
add_test(NAME hash_table_test COMMAND $<TARGET_FILE:hash_table_test>)

add_executable(hash_table_bad_hash_test src-tests/hash_table.c)
target_compile_definitions(hash_table_bad_hash_test PUBLIC USE_BAD_HASH_FUNCTION)
add_test(NAME hash_table_bad_hash_test COMMAND $<TARGET_FILE:hash_table_bad_hash_test>)

add_executable(parse_line_test src-tests/parse_line.c)
add_test(NAME parse_line_test COMMAND $<TARGET_FILE:parse_line_test>)
