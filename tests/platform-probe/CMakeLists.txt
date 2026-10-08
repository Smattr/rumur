# this is only relevant if we are targeting the host machine
if(NOT CMAKE_CROSSCOMPILING)
  add_executable(platform-probe main.c)
endif()
