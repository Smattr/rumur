#include <iostream>
#include <rumur/Property.h>

namespace rumur {

Property::Property(Category category_, Expr *expr_, const location &loc_):
  Node(loc_), category(category_), expr(expr_) { }

Property::Property(const Property &other):
  Node(other.loc), category(other.category), expr(other.expr->clone()) { }

Property &Property::operator=(Property other) {
  swap(*this, other);
  return *this;
}

void swap(Property &x, Property &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.category, y.category);
  swap(x.expr, y.expr);
}

Property *Property::clone() const {
  return new Property(*this);
}

Property::~Property() {
  delete expr;
}

void Property::generate(std::ostream &out) const {
  out << *expr;
}

bool Property::operator==(const Node &other) const {
  auto o = dynamic_cast<const Property*>(&other);
  return o != nullptr && category == o->category && *expr == *o->expr;
}

}
