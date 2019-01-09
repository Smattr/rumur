add_executable(rumur-ast-dump
  src/main.cc
  src/XMLPrinter.cc)

target_include_directories(rumur-ast-dump
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(rumur-ast-dump
  librumur)

# Compress manpages
add_custom_target(man-ast-dump
  ALL
  DEPENDS rumur-ast-dump.1.gz)
add_custom_command(
  OUTPUT rumur-ast-dump.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/rumur-ast-dump.1 >"${CMAKE_CURRENT_BINARY_DIR}/rumur-ast-dump.1.gz"
  MAIN_DEPENDENCY doc/rumur-ast-dump.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS rumur-ast-dump
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/rumur-ast-dump.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
