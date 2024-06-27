# Define resources.
add_custom_command(
  OUTPUT resources_manpage.cc
  COMMAND ../misc/xxd.py doc/rumur.1 ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.cc
  MAIN_DEPENDENCY doc/rumur.1
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
  OUTPUT resources_includes.cc
  COMMAND ../misc/xxd.py resources/includes.c ${CMAKE_CURRENT_BINARY_DIR}/resources_includes.cc
  MAIN_DEPENDENCY resources/includes.c
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
  OUTPUT resources_header.cc
  COMMAND ../misc/xxd.py resources/header.c ${CMAKE_CURRENT_BINARY_DIR}/resources_header.cc
  MAIN_DEPENDENCY resources/header.c
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(rumur
  ${CMAKE_CURRENT_BINARY_DIR}/resources_includes.cc
  ${CMAKE_CURRENT_BINARY_DIR}/resources_header.cc
  ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.cc
  ../common/escape.cc
  ../common/help.cc
  src/assume-statements-count.cc
  src/generate-allocations.cc
  src/generate-cover-array.cc
  src/generate-decl.cc
  src/generate-expr.cc
  src/generate-function.cc
  src/generate-model.cc
  src/generate-print.cc
  src/generate-property.cc
  src/generate-quantifier.cc
  src/generate-stmt.cc
  src/has-start-state.cc
  src/log.cc
  src/main.cc
  src/max-simple-width.cc
  src/optimise-field-ordering.cc
  src/options.cc
  src/output.cc
  src/prints-scalarsets.cc
  src/process.cc
  src/smt/define-enum-members.cc
  src/smt/define-records.cc
  src/smt/logic.cc
  src/smt/simplify.cc
  src/smt/solver.cc
  src/smt/translate.cc
  src/smt/typeexpr-to-smt.cc
  src/symmetry-reduction.cc
  src/utils.cc
  src/ValueType.cc)

target_include_directories(rumur
  PRIVATE
  src
  # FIXME: This is a hack to include generated headers from the library. We
  # really want to be able to stage these somewhere and talk about them here
  # as if they were just a regular, static exported header.
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(rumur
  PRIVATE
  librumur)

# Compress manpages
add_custom_target(man-rumur
  ALL
  DEPENDS rumur.1.gz rumur-run.1.gz)
add_custom_command(
  OUTPUT rumur.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/rumur.1 >"${CMAKE_CURRENT_BINARY_DIR}/rumur.1.gz"
  MAIN_DEPENDENCY doc/rumur.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
  OUTPUT rumur-run.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/rumur-run.1 >"${CMAKE_CURRENT_BINARY_DIR}/rumur-run.1.gz"
  MAIN_DEPENDENCY doc/rumur-run.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS rumur
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/rumur-run
  DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/rumur.1.gz
  ${CMAKE_CURRENT_BINARY_DIR}/rumur-run.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
