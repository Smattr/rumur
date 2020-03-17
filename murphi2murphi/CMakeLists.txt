add_executable(murphi2murphi
  src/ExplicitSemicolons.cc
  src/main.cc
  src/options.cc
  src/Pipeline.cc
  src/Printer.cc
  src/RemoveLiveness.cc
  src/Stage.cc
  src/SwitchToIf.cc)

target_include_directories(murphi2murphi
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(murphi2murphi
  librumur)

install(TARGETS murphi2murphi
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
