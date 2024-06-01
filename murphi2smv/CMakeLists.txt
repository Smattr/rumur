add_custom_command(
  OUTPUT manpage.cc
  COMMAND ../misc/xxd.py doc/murphi2smv.1 ${CMAKE_CURRENT_BINARY_DIR}/manpage.cc
  MAIN_DEPENDENCY doc/murphi2smv.1
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(murphi2smv
  ${CMAKE_CURRENT_BINARY_DIR}/manpage.cc
  ../common/help.cc
  src/codegen.cc
  src/main.cc
  src/pick_numeric_type.cc
)

target_include_directories(murphi2smv PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur
)

target_link_libraries(murphi2smv PRIVATE librumur)

add_custom_target(man-murphi2smv ALL DEPENDS murphi2smv.1.gz)
add_custom_command(
  OUTPUT murphi2smv.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/murphi2smv.1 >"${CMAKE_CURRENT_BINARY_DIR}/murphi2smv.1.gz"
  MAIN_DEPENDENCY doc/murphi2smv.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS murphi2smv
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/murphi2smv.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
