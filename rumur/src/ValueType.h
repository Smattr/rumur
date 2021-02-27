#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <rumur/rumur.h>
#include <string>
#include <utility>

// abstraction over the type used to represent scalar values during checking
struct ValueType {
  std::string c_type;  // C symbol that names the type
  std::string int_min; // equivalent of INT_MIN
  std::string int_max; // equivalent of INT_MAX
  std::string int_c;   // equivalent of INT_C
  std::string pri;     // equivalent of PRId64
  mpz_class min;       // minimum value that can be represented in this type
  mpz_class max;       // maximum value that can be represented in this type
};

std::pair<ValueType, ValueType> get_value_type(const std::string &name,
                                               const rumur::Model &m);
