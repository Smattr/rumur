# Define resources.
add_custom_command(
  OUTPUT resources_manpage.cc
  COMMAND ../misc/xxd.py doc/murphi2murphi.1 ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.cc
  MAIN_DEPENDENCY doc/murphi2murphi.1
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(murphi2murphi
  ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.cc
  ../common/help.cc
  src/DecomposeComplexComparisons.cc
  src/ExplicitSemicolons.cc
  src/main.cc
  src/options.cc
  src/Pipeline.cc
  src/Printer.cc
  src/RemoveLiveness.cc
  src/Stage.cc
  src/SwitchToIf.cc
  src/ToAscii.cc)

target_include_directories(murphi2murphi
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(murphi2murphi
  PRIVATE
  librumur)

# Compress manpage
add_custom_target(man-murphi2murphi
  ALL
  DEPENDS murphi2murphi.1.gz)
add_custom_command(
  OUTPUT murphi2murphi.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/murphi2murphi.1 >"${CMAKE_CURRENT_BINARY_DIR}/murphi2murphi.1.gz"
  MAIN_DEPENDENCY doc/murphi2murphi.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS murphi2murphi
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/murphi2murphi.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
