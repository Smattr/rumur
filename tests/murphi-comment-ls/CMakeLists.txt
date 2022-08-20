# this is only usable for testing if we are targeting the host machine
if(NOT CMAKE_CROSSCOMPILING)
  add_executable(murphi-comment-ls main.cc)
  target_include_directories(murphi-comment-ls PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../librumur)
  target_link_libraries(murphi-comment-ls PRIVATE librumur)
endif()
