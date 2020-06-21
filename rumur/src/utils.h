#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <rumur/rumur.h>
#include <string>

template<typename T, typename U>
bool isa(const U ptr) {
  return ptr != nullptr && dynamic_cast<const T*>(&*ptr) != nullptr;
}

// get a C source code string for this expression
std::string to_C_string(const rumur::Expr &expr);

// get a C source code string for this location
std::string to_C_string(const rumur::location &location);

// how many bits are required to store `v` unique values?
mpz_class bit_width(const mpz_class &v);
