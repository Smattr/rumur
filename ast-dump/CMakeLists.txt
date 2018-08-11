add_executable(rumur-ast-dump
  src/main.cc
  src/XMLPrinter.cc)

target_include_directories(rumur-ast-dump
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(rumur-ast-dump
  librumur)

install(TARGETS rumur-ast-dump
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
