# this is only usable for testing if we are targeting the host machine
if(NOT CMAKE_CROSSCOMPILING)
  add_executable(union-array-width main.cc)
  target_include_directories(union-array-width PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../librumur)
  target_link_libraries(union-array-width PRIVATE librumur)
  target_include_directories(union-array-width SYSTEM PRIVATE ${GMPXX_INCLUDE})
  target_link_libraries(union-array-width
    PRIVATE ${GMPXX_LIBRARIES} ${GMP_LIBRARIES}
  )
endif()
