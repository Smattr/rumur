add_executable(rumur
    src/main.cc)

target_include_directories(rumur
    PRIVATE
    # FIXME: This is a hack to include generated headers from the library. We
    # really want to be able to stage these somewhere and talk about them here
    # as if they were just a regular, static exported header.
    ${CMAKE_CURRENT_BINARY_DIR}/../librumur)

target_link_libraries(rumur
    librumur)

install(TARGETS rumur
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
