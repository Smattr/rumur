#pragma once

#include <cstdint>
#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <string>

namespace rumur {

struct Number : public Expr {

  int64_t value;

  Number() = delete;
  Number(const std::string &value_, const location &loc_);
  Number(int64_t value_, const location &loc_);
  Number(const Number&) = default;
  Number(Number&&) = default;
  Number &operator=(const Number&) = default;
  Number &operator=(Number&&) = default;
  virtual ~Number() { }
  Number *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

}
