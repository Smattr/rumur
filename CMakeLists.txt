cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(rumur LANGUAGES CXX)

# This seems to be some magic to get libraries to install correctly.
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wformat=2 \
  -Wwrite-strings -Wmissing-declarations -Wshadow -Wundef")

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
CHECK_CXX_COMPILER_FLAG(-Wlogical-op HAS_WARNING_LOGICAL_OP)
if(HAS_WARNING_LOGICAL_OP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
endif()
CHECK_CXX_COMPILER_FLAG(-Wstrict-aliasing=1 HAS_WARNING_STRICT_ALIASING_1)
if(HAS_WARNING_STRICT_ALIASING_1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-aliasing=1")
endif()
CHECK_CXX_COMPILER_FLAG(-Wpointer-arith HAS_WARNING_POINTER_ARITH)
if(HAS_WARNING_POINTER_ARITH)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith")
endif()

# make mkostemp(), pipe2() prototype visible
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_definitions(-D_GNU_SOURCE)
endif()

# Enable --as-needed, present on GNU ld on Linux, to minimise dependencies.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
endif()

if(APPLE)
  list(APPEND CMAKE_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}")
else()
  list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

# if we have a new enough CMake to have FindPython3, check for it
if(CMAKE_VERSION VERSION_GREATER 3.11)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
endif()

add_subdirectory(librumur)
add_subdirectory(murphi2c)
add_subdirectory(murphi2murphi)
add_subdirectory(murphi2smv)
add_subdirectory(murphi2uclid)
add_subdirectory(murphi2xml)
add_subdirectory(rumur)
add_subdirectory(tests/murphi-comment-ls)

add_custom_target(check
  COMMAND env PATH=${CMAKE_CURRENT_BINARY_DIR}/rumur:${CMAKE_CURRENT_BINARY_DIR}/murphi2c:${CMAKE_CURRENT_BINARY_DIR}/murphi2murphi:${CMAKE_CURRENT_BINARY_DIR}/murphi2uclid:${CMAKE_CURRENT_BINARY_DIR}/murphi2xml:${CMAKE_CURRENT_BINARY_DIR}/tests/murphi-comment-ls:$ENV{PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-tests.py --verbose)
add_dependencies(check murphi2c murphi2murphi murphi2uclid murphi2xml rumur)
if(NOT CMAKE_CROSSCOMPILING)
  add_dependencies(check murphi-comment-ls)
endif()

add_custom_target(format
  COMMAND git ls-files -z '**/*.cc' '**/*.h' |
    xargs -0 -- clang-format -i --style=file
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
