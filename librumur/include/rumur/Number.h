#pragma once

#include "location.hh"
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <rumur/Expr.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <string>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

struct RUMUR_API_WITH_RTTI Number : public Expr {

  mpz_class value;

  Number() = delete;
  Number(const std::string &value_, const location &loc_);
  Number(const mpz_class &value_, const location &loc_);
  Number(const Number &) = default;
  Number(Number &&) = default;
  Number &operator=(const Number &) = default;
  Number &operator=(Number &&) = default;
  virtual ~Number() = default;
  Number *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  std::string to_string() const override;
  bool is_pure() const override;
};

} // namespace rumur
