cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(rumur LANGUAGES CXX)

# This seems to be some magic to get libraries to install correctly.
include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wextra -Wwrite-strings -Wmissing-declarations -Wshadow")

# enable even more warnings if the compiler supports them
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-Wcast-qual HAS_WARNING_CAST_QUAL)
if(HAS_WARNING_CAST_QUAL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual")
endif()
CHECK_CXX_COMPILER_FLAG(-Wcast-align HAS_WARNING_CAST_ALIGN)
if(HAS_WARNING_CAST_ALIGN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align")
endif()
CHECK_CXX_COMPILER_FLAG(-Wstrict-aliasing=1 HAS_WARNING_STRICT_ALIASING_1)
if(HAS_WARNING_STRICT_ALIASING_1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-aliasing=1")
endif()
CHECK_CXX_COMPILER_FLAG(-Wpointer-arith HAS_WARNING_POINTER_ARITH)
if(HAS_WARNING_POINTER_ARITH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith")
endif()

# Enable --as-needed, present on GNU ld on Linux, to minimise dependencies.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
endif()

# if we have a new enough CMake to have FindPython3, check for it
if(${CMAKE_VERSION} VERSION_GREATER 3.11)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
endif()

add_subdirectory(librumur)
add_subdirectory(murphi2c)
add_subdirectory(murphi2murphi)
add_subdirectory(murphi2xml)
add_subdirectory(rumur)

add_custom_target(check
  COMMAND env PATH=${CMAKE_CURRENT_BINARY_DIR}/rumur:${CMAKE_CURRENT_BINARY_DIR}/murphi2c:${CMAKE_CURRENT_BINARY_DIR}/murphi2murphi:${CMAKE_CURRENT_BINARY_DIR}/murphi2xml:$ENV{PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-tests.py --jobs 1)
add_dependencies(check murphi2c murphi2murphi murphi2xml rumur)
