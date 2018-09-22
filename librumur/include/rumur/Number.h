#pragma once

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <string>

namespace rumur {

struct Number : public Expr {

  mpz_class value;

  Number() = delete;
  Number(const std::string &value_, const location &loc_);
  Number(const mpz_class &value_, const location &loc_);
  Number(const Number&) = default;
  Number(Number&&) = default;
  Number &operator=(const Number&) = default;
  Number &operator=(Number&&) = default;
  virtual ~Number() { }
  Number *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

}
