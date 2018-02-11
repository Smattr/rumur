#include <cassert>
#include <cstdint>
#include <iostream>
#include "location.hh"
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Number.h>
#include <stdexcept>
#include <string>

namespace rumur {

static_assert(sizeof(long long) >= sizeof(int64_t),
  "a 64-bit integer cannot be read with stoll");

Number::Number(const std::string &value_, const location &loc_):
  Expr(loc_) {
  try {
    value = int64_t(stoll(value_, nullptr, 0));
  } catch (std::invalid_argument &e) {
    throw Error("invalid number: " + value_, loc);
  } catch (std::out_of_range &e) {
    throw Error("out of range number: " + value_, loc);
  }
}

Number::Number(int64_t value_, const location &loc_):
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

void Number::generate(std::ostream &out) const {
  out << "Number(INT64_C(" << value << "))";
}

int64_t Number::constant_fold() const {
  return value;
}

bool Number::operator==(const Node &other) const {
  auto o = dynamic_cast<const Number*>(&other);
  return o != nullptr && value == o->value;
}

}
