add_library(util OBJECT
  src/escape.cc
  src/help.cc)

target_include_directories(util
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
  src)
