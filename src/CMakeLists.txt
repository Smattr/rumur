cmake_minimum_required(VERSION 3.1)

project(rumur LANGUAGES CXX)

# This seems to be some magic to get libraries to install correctly.
include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wextra -Wwrite-strings -Wmissing-declarations -Wshadow")

add_subdirectory(librumur)
add_subdirectory(rumur)

enable_testing()
add_test(integration ${CMAKE_CURRENT_SOURCE_DIR}/../tests/integration-tests.py)
