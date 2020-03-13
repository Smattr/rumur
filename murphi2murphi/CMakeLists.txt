add_executable(murphi2murphi
  src/find-liveness.cc
  src/main.cc
  src/options.cc
  src/Printer.cc)

target_include_directories(murphi2murphi
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(murphi2murphi
  librumur)

install(TARGETS murphi2murphi
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
