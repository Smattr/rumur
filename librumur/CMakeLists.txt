find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

find_path(GMPXX_INCLUDE NAMES gmpxx.h PATHS ENV CPLUS_INCLUDE_PATH)
if(NOT GMPXX_INCLUDE)
  message(FATAL_ERROR "GMP headers not found")
endif()
find_library(GMP_LIBRARIES NAMES gmp PATHS ENV LIBRARY_PATH)
if(NOT GMP_LIBRARIES)
  message(FATAL_ERROR "libgmp not found")
endif()
find_library(GMPXX_LIBRARIES NAMES gmpxx PATHS ENV LIBRARY_PATH)
if(NOT GMPXX_LIBRARIES)
  message(FATAL_ERROR "libgmpxx not found")
endif()

bison_target(parser
  src/parser.yy
  ${CMAKE_CURRENT_BINARY_DIR}/parser.yy.cc
  COMPILE_FLAGS "--no-lines --warnings=all")
flex_target(lexer
  src/lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.l.cc
  COMPILE_FLAGS "--noline")
add_flex_bison_dependency(lexer parser)

# Suppress warnings in generated code.
include(CheckCXXCompilerFlag)
set_source_files_properties(lexer.l.cc PROPERTIES COMPILE_FLAGS "")
CHECK_CXX_COMPILER_FLAG(-Wsign-compare HAS_WARNING_SIGN_COMPARE)
if(HAS_WARNING_SIGN_COMPARE)
  set_source_files_properties(lexer.l.cc PROPERTIES COMPILE_FLAGS "${LEXER_COMPILE_FLAGS} -Wno-sign-compare")
endif()
get_source_file_property(LEXER_COMPILE_FLAGS lexer.l.cc COMPILE_FLAGS)
CHECK_CXX_COMPILER_FLAG(-Wregister HAS_WARNING_REGISTER)
if(HAS_WARNING_REGISTER)
  set_source_files_properties(lexer.l.cc PROPERTIES COMPILE_FLAGS "${LEXER_COMPILE_FLAGS} -Wno-register")
endif()
get_source_file_property(LEXER_COMPILE_FLAGS lexer.l.cc COMPILE_FLAGS)
CHECK_CXX_COMPILER_FLAG(-Wdeprecated-register HAS_WARNING_DEPRECATED_REGISTER)
if(HAS_WARNING_DEPRECATED_REGISTER)
  set_source_files_properties(lexer.l.cc PROPERTIES COMPILE_FLAGS "${LEXER_COMPILE_FLAGS} -Wno-deprecated-register")
endif()

add_custom_command(
  OUTPUT rumur-get-version.h
  COMMAND src/make-version.py ${CMAKE_CURRENT_BINARY_DIR}/rumur-get-version.h
  MAIN_DEPENDENCY src/make-version.py
  DEPENDS always_run
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Dummy output to make sure we always re-evaluate the version step above.
add_custom_command(
  OUTPUT always_run
  COMMAND /usr/bin/env true)

add_library(librumur
  ${CMAKE_CURRENT_BINARY_DIR}/rumur-get-version.h
  src/Boolean.cc
  src/Comment.cc
  src/Decl.cc
  src/except.cc
  src/Expr.cc
  src/Function.cc
  src/indexer.cc
  src/Model.cc
  src/Node.cc
  src/Number.cc
  src/parse.cc
  src/Property.cc
  src/resolve-symbols.cc
  src/Rule.cc
  src/sanitise_rule_names.cc
  src/Stmt.cc
  src/traverse.cc
  src/TypeExpr.cc
  src/validate.cc
  ${FLEX_lexer_OUTPUTS}
  ${BISON_parser_OUTPUTS})

target_include_directories(librumur
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(librumur
  SYSTEM PUBLIC
  ${GMPXX_INCLUDE}
)
target_link_libraries(librumur PUBLIC ${GMPXX_LIBRARIES} ${GMP_LIBRARIES})

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
  ${CMAKE_CURRENT_BINARY_DIR}/rumur-get-version.h
  ${CMAKE_CURRENT_BINARY_DIR}/stack.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rumur)

export(TARGETS librumur FILE LibrumurConfig.cmake)
