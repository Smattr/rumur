cmake_minimum_required(VERSION 3.1)

project(rumur CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wwrite-strings -std=c++17")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(parser
    parser.yy
    ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cc)
flex_target(lexer
    lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.l.cc)
add_flex_bison_dependency(lexer parser)

# Suppress warnings in generated code.
set_source_files_properties(lexer.l.cc PROPERTIES COMPILE_FLAGS -Wno-sign-compare)
set_source_files_properties(lexer.l.cc PROPERTIES COMPILE_FLAGS -Wno-register)

# Define resources.
add_custom_command(
    OUTPUT resources_header.cc
    COMMAND xxd -include resources/header.cc ${CMAKE_CURRENT_BINARY_DIR}/resources_header.cc
    MAIN_DEPENDENCY resources/header.cc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_custom_command(
    OUTPUT resources_includes.cc
    COMMAND xxd -include resources/includes.cc ${CMAKE_CURRENT_BINARY_DIR}/resources_includes.cc
    MAIN_DEPENDENCY resources/includes.cc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(rumur
    Decl.cc
    main.cc
    Model.cc
    Number.cc
    output.cc
    ${CMAKE_CURRENT_BINARY_DIR}/resources_header.cc
    ${CMAKE_CURRENT_BINARY_DIR}/resources_includes.cc
    ${FLEX_lexer_OUTPUTS}
    ${BISON_parser_OUTPUTS})
target_include_directories(rumur
    PRIVATE
    .
    ${CMAKE_CURRENT_BINARY_DIR})
