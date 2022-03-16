#pragma once

#include "location.hh"
#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

struct RUMUR_API_WITH_RTTI Property : public Node {

  enum Category {
    ASSERTION,
    ASSUMPTION,
    COVER,
    LIVENESS,
  };

  Category category;
  Ptr<Expr> expr;

  Property(Category category_, const Ptr<Expr> &expr_, const location &loc_);
  Property *clone() const final;
  virtual ~Property() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;
};

} // namespace rumur
