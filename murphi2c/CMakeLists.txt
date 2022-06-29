# Define resources.
add_custom_command(
  OUTPUT resources_manpage.cc
  COMMAND ../misc/xxd.py doc/murphi2c.1 ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.cc
  MAIN_DEPENDENCY doc/murphi2c.1
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
  OUTPUT resources_c_prefix.cc
  COMMAND ../misc/xxd.py resources/c_prefix.c ${CMAKE_CURRENT_BINARY_DIR}/resources_c_prefix.cc
  MAIN_DEPENDENCY resources/c_prefix.c
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(
  OUTPUT resources_h_prefix.cc
  COMMAND ../misc/xxd.py resources/h_prefix.h ${CMAKE_CURRENT_BINARY_DIR}/resources_h_prefix.cc
  MAIN_DEPENDENCY resources/h_prefix.h
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(murphi2c
  ${CMAKE_CURRENT_BINARY_DIR}/resources_c_prefix.cc
  ${CMAKE_CURRENT_BINARY_DIR}/resources_h_prefix.cc
  ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.cc
  ../common/escape.cc
  ../common/help.cc
  src/check.cc
  src/CLikeGenerator.cc
  src/CodeGenerator.cc
  src/compares_complex_values.cc
  src/generate_c.cc
  src/generate_h.cc
  src/main.cc
  src/options.cc)

target_include_directories(murphi2c
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(murphi2c
  PRIVATE
  librumur)

# Compress manpage
add_custom_target(man-murphi2c
  ALL
  DEPENDS murphi2c.1.gz)
add_custom_command(
  OUTPUT murphi2c.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/murphi2c.1 >"${CMAKE_CURRENT_BINARY_DIR}/murphi2c.1.gz"
  MAIN_DEPENDENCY doc/murphi2c.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS murphi2c
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/murphi2c.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
