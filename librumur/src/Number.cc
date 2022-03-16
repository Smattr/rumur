#include "location.hh"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <rumur/Expr.h>
#include <rumur/Number.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <rumur/except.h>
#include <rumur/traverse.h>
#include <stdexcept>
#include <string>

namespace rumur {

Number::Number(const std::string &value_, const location &loc_) try
    : Expr(loc_), value(value_) {
} catch (std::invalid_argument &e) {
  throw Error("invalid number: " + value_, loc_);
}

Number::Number(const mpz_class &value_, const location &loc_)
    : Expr(loc_), value(value_) {}

Number *Number::clone() const { return new Number(*this); }

void Number::visit(BaseTraversal &visitor) { visitor.visit_number(*this); }

void Number::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_number(*this);
}

bool Number::constant() const { return true; }

Ptr<TypeExpr> Number::type() const {
  return Ptr<Range>::make(nullptr, nullptr, location());
}

mpz_class Number::constant_fold() const { return value; }

std::string Number::to_string() const { return value.get_str(); }

bool Number::is_pure() const { return true; }

} // namespace rumur
