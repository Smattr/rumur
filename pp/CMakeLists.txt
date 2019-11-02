add_executable(rumur-pp
  src/main.cc)

target_include_directories(rumur-pp
  PRIVATE
  src
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(rumur-pp
  librumur)

install(TARGETS rumur-pp
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
