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
    DISABLED,
    ASSERTION,
    ASSUMPTION,
  };

  Category category;
  Ptr<Expr> expr;

  Property() = delete;
  Property(Category category_, const Ptr<Expr> &expr_,
    const location &loc_);
  Property(const Property &other);
  Property &operator=(Property other);
  friend void swap(Property &x, Property &y) noexcept;
  Property *clone() const final;
  virtual ~Property() = default;

  bool operator==(const Node &other) const final;
};

}
