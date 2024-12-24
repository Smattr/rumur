add_custom_command(
  OUTPUT resources_manpage.c
  COMMAND ../misc/xxd.py doc/murphi-format.1 ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.c
  MAIN_DEPENDENCY doc/murphi-format.1
  DEPENDS ../misc/xxd.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(murphi-format
  ${CMAKE_CURRENT_BINARY_DIR}/resources_manpage.c
  ../common/help.c
  src/format.c
  src/lex_free.c
  src/lex_get_token.c
  src/lex_new.c
  src/main.c
)

target_include_directories(murphi-format
  PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/../librumur
)
target_link_libraries(murphi-format PRIVATE librumur)

add_custom_target(man-murphi-format
  ALL
  DEPENDS murphi-format.1.gz)
add_custom_command(
  OUTPUT murphi-format.1.gz
  COMMAND gzip -9 --no-name --to-stdout doc/murphi-format.1 >"${CMAKE_CURRENT_BINARY_DIR}/murphi-format.1.gz"
  MAIN_DEPENDENCY doc/murphi-format.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS murphi-format
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/murphi-format.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
