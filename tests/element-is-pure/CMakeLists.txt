# this is only usable for testing if we are targeting the host machine
if(NOT CMAKE_CROSSCOMPILING)
  add_executable(element-is-pure main.cc)
  target_include_directories(element-is-pure PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../librumur)
  target_link_libraries(element-is-pure PRIVATE librumur)
endif()
