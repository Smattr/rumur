#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <string>

namespace smt {

class Logic {

 public:
  virtual std::string integer_type() const = 0;

  virtual std::string numeric_literal(const mpz_class &value) const = 0;

  virtual std::string add(void) const = 0;
  virtual std::string div(void) const = 0;
  virtual std::string geq(void) const = 0;
  virtual std::string gt(void) const = 0;
  virtual std::string leq(void) const = 0;
  virtual std::string lt(void) const = 0;
  virtual std::string mod(void) const = 0;
  virtual std::string mul(void) const = 0;
  virtual std::string neg(void) const = 0;
  virtual std::string sub(void) const = 0;
};

// find the version of the above API for a given SMT logic
const Logic &get_logic(const std::string &name);

}
