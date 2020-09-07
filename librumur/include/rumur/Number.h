#pragma once

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
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
  virtual ~Number() = default;
  Number *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
  bool is_pure() const final;
};

}
