add_custom_command(
  OUTPUT manpage.cc
  COMMAND ../misc/xxd.py doc/murphi2xml.1 ${CMAKE_CURRENT_BINARY_DIR}/manpage.cc
  MAIN_DEPENDENCY doc/murphi2xml.1
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(murphi2xml
  ${CMAKE_CURRENT_BINARY_DIR}/manpage.cc
  ../common/help.cc
  src/main.cc
  src/XMLPrinter.cc)

target_include_directories(murphi2xml
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(murphi2xml
  PRIVATE
  librumur)

# Compress manpages
add_custom_target(man-murphi2xml
  ALL
  DEPENDS murphi2xml.1.gz)
add_custom_command(
  OUTPUT murphi2xml.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/murphi2xml.1 >"${CMAKE_CURRENT_BINARY_DIR}/murphi2xml.1.gz"
  MAIN_DEPENDENCY doc/murphi2xml.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS murphi2xml
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/murphi2xml.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
