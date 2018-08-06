#pragma once

#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>

namespace rumur {

struct Property : public Node {

  enum Category {
    DISABLED,
    ASSERTION,
    ASSUMPTION,
  };

  Category category;
  Expr *expr;

  Property() = delete;
  Property(Category category_, Expr *expr_, const location &loc_);
  Property(const Property &other);
  Property &operator=(Property other);
  friend void swap(Property &x, Property &y) noexcept;
  Property *clone() const final;
  virtual ~Property();

  void generate(std::ostream &out) const;
  bool operator==(const Node &other) const final;
};

}
