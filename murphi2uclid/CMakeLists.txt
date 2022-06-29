add_custom_command(
  OUTPUT manpage.cc
  COMMAND ../misc/xxd.py doc/murphi2uclid.1 ${CMAKE_CURRENT_BINARY_DIR}/manpage.cc
  MAIN_DEPENDENCY doc/murphi2uclid.1
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(murphi2uclid
  ${CMAKE_CURRENT_BINARY_DIR}/manpage.cc
  ../common/help.cc
  src/check.cc
  src/codegen.cc
  src/main.cc
  src/pick_numeric_type.cc)

target_include_directories(murphi2uclid
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(murphi2uclid
  PRIVATE
  librumur)

add_custom_target(man-murphi2uclid
  ALL
  DEPENDS murphi2uclid.1.gz)
add_custom_command(
  OUTPUT murphi2uclid.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/murphi2uclid.1 >"${CMAKE_CURRENT_BINARY_DIR}/murphi2uclid.1.gz"
  MAIN_DEPENDENCY doc/murphi2uclid.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS murphi2uclid
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/murphi2uclid.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
