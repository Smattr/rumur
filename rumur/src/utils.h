#pragma once

#include <cstddef>
#include <string>

template<typename T, typename U>
bool isa(const U ptr) {
  return ptr != nullptr && dynamic_cast<const T*>(&*ptr) != nullptr;
}

// escape a string for the purposes of outputting it to a C source file
std::string escape(const std::string &s);
