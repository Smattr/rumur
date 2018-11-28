#include <cassert>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Number.h>
#include <stdexcept>
#include <string>

namespace rumur {

Number::Number(const std::string &value_, const location &loc_) try:
   Expr(loc_), value(value_) {
} catch (std::invalid_argument &e) {
  throw Error("invalid number: " + value_, loc_);
}

Number::Number(const mpz_class &value_, const location &loc_):
  Expr(loc_), value(value_) {
}

Number *Number::clone() const {
  return new Number(*this);
}

bool Number::constant() const {
  return true;
}

const TypeExpr *Number::type() const {
  return nullptr;
}

mpz_class Number::constant_fold() const {
  return value;
}

bool Number::operator==(const Node &other) const {
  auto o = dynamic_cast<const Number*>(&other);
  return o != nullptr && value == o->value;
}

std::string Number::to_string() const {
  return value.get_str();
}

}
