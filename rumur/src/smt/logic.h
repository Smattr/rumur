#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <string>

namespace smt {

class Logic {

 private:
  bool arrays;
  bool bitvectors;
  bool integers;

 public:
  Logic(bool a, bool bv, bool ia);

  std::string integer_type() const;

  std::string numeric_literal(const mpz_class &value) const;

  std::string add(void) const;
  std::string div(void) const;
  std::string geq(void) const;
  std::string gt (void) const;
  std::string leq(void) const;
  std::string lt (void) const;
  std::string mod(void) const;
  std::string mul(void) const;
  std::string neg(void) const;
  std::string sub(void) const;

  bool supports_arrays() const;
};

// find the version of the above API for a given SMT logic
const Logic &get_logic(const std::string &name);

}
