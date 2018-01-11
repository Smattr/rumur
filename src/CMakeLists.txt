cmake_minimum_required(VERSION 3.1)

project(rumur LANGUAGES CXX)

# This seems to be some magic to get libraries to install correctly.
include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wextra -Wwrite-strings -Wmissing-declarations")

# Enable colour diagnostic messages. This is often enabled by default or by
# environment variables. However, this behaviour is suppressed when buliding
# with Ninja because it redirects the compiler's output to a pipe. To get
# colours back we need to force them on.
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color")
endif()

add_subdirectory(librumur)
add_subdirectory(rumur)
