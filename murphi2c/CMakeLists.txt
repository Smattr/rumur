add_executable(murphi2c
  src/check.cc
  src/generate_c.cc
  src/main.cc
  src/name_rules.cc)

target_include_directories(murphi2c
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(murphi2c
  librumur)

install(TARGETS murphi2c
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
