#pragma once

#include <cstddef>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>

namespace rumur {

struct Property : public Node {

  enum Category {
    ASSERTION,
    ASSUMPTION,
    COVER,
    LIVENESS,
  };

  Category category;
  Ptr<Expr> expr;

  Property(Category category_, const Ptr<Expr> &expr_,
    const location &loc_);
  Property *clone() const final;
  virtual ~Property() = default;

  __attribute__((deprecated("operator== will be removed in a future release")))
  bool operator==(const Node &other) const final;
};

}
