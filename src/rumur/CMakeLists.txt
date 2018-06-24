if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  # GNU `man` wants this extra option
  set (MANFLAGS "--local-file")
endif ()

# Generate `man` output that we'll embed into the binary.
add_custom_command(
  OUTPUT rumur.1
  COMMAND cp -f "${CMAKE_CURRENT_SOURCE_DIR}/doc/rumur.1" "${CMAKE_CURRENT_BINARY_DIR}/rumur.1"
  MAIN_DEPENDENCY doc/rumur.1)
add_custom_command(
  OUTPUT resources_manpage.text
  COMMAND man ${MANFLAGS} ./rumur.1 >resources_manpage.text
  MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/rumur.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(
  OUTPUT resources_manpage.cc
  COMMAND xxd -include resources_manpage.text ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.cc
  MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.text
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(rumur
  resources_manpage.cc
  src/main.cc)

target_include_directories(rumur
  PRIVATE
  src
  # FIXME: This is a hack to include generated headers from the library. We
  # really want to be able to stage these somewhere and talk about them here
  # as if they were just a regular, static exported header.
  ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(rumur
  librumur)

install(TARGETS rumur
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
