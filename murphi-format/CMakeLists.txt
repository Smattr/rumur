add_executable(murphi-format
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

install(TARGETS murphi-format
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
