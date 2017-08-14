cmake_minimum_required(VERSION 3.1)

project(rumur LANGUAGES CXX)

# This seems to be some magic to get libraries to install correctly.
include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wwrite-strings -Wmissing-declarations")

# Use C++17. In Clang, this is called C++1z because it isn't ratified yet. At
# time of writing, no Clang release supports template argument deductions which
# are used in the libstdc++ headers that accompany GCC. If you want to build
# using this combination, you will have to use the Clang SVN trunk.
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

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
