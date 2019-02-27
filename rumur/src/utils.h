#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <string>

template<typename T, typename U>
bool isa(const U ptr) {
  return ptr != nullptr && dynamic_cast<const T*>(&*ptr) != nullptr;
}

// escape a string for the purposes of outputting it to a C source file
std::string escape(const std::string &s);

// get a C source code string for this expression
std::string to_C_string(const rumur::Expr &expr);
