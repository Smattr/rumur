cmake_minimum_required(VERSION 3.1)

project(rumur LANGUAGES CXX)

# This seems to be some magic to get libraries to install correctly.
include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wwrite-strings -Wmissing-declarations -std=c++17")

add_subdirectory(librumur)
add_subdirectory(rumur)
