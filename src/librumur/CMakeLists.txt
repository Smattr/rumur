project(librumur LANGUAGES CXX)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(parser
    src/parser.yy
    ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cc
    COMPILE_FLAGS --warnings=all)
flex_target(lexer
    src/lexer.l
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
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
    OUTPUT resources_main.cc
    COMMAND xxd -include resources/main.cc ${CMAKE_CURRENT_BINARY_DIR}/resources_main.cc
    MAIN_DEPENDENCY resources/main.cc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
    OUTPUT resources_Rule.cc
    COMMAND xxd -include resources/Rule.cc ${CMAKE_CURRENT_BINARY_DIR}/resources_Rule.cc
    MAIN_DEPENDENCY resources/Rule.cc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
    OUTPUT resources_State.cc
    COMMAND xxd -include resources/State.cc ${CMAKE_CURRENT_BINARY_DIR}/resources_State.cc
    MAIN_DEPENDENCY resources/State.cc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(librumur
    src/Boolean.cc
    src/Decl.cc
    src/except.cc
    src/Expr.cc
    src/Indexer.cc
    src/Model.cc
    src/Node.cc
    src/Number.cc
    src/output.cc
    src/parse.cc
    src/Rule.cc
    src/Stmt.cc
    src/TypeExpr.cc
    ${CMAKE_CURRENT_BINARY_DIR}/resources_header.cc
    ${CMAKE_CURRENT_BINARY_DIR}/resources_main.cc
    ${CMAKE_CURRENT_BINARY_DIR}/resources_Rule.cc
    ${CMAKE_CURRENT_BINARY_DIR}/resources_State.cc
    ${FLEX_lexer_OUTPUTS}
    ${BISON_parser_OUTPUTS})

target_include_directories(librumur
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR})

# Force the output to librumur.a instead of liblibrumur.a.
set_target_properties(librumur PROPERTIES PREFIX "")

# Clagged boiler plate to install a library.
install(TARGETS librumur EXPORT LibrumurConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/location.hh
    ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.hh
    ${CMAKE_CURRENT_BINARY_DIR}/position.hh
    ${CMAKE_CURRENT_BINARY_DIR}/stack.hh
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rumur)

export(TARGETS librumur FILE LibrumurConfig.cmake)
