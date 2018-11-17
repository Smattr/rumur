#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/Property.h>
#include <rumur/Ptr.h>

namespace rumur {

Property::Property(Category category_, const Ptr<Expr> &expr_,
  const location &loc_):
  Node(loc_), category(category_), expr(expr_) { }

Property *Property::clone() const {
  return new Property(*this);
}

bool Property::operator==(const Node &other) const {
  auto o = dynamic_cast<const Property*>(&other);
  return o != nullptr && category == o->category && *expr == *o->expr;
}

}
