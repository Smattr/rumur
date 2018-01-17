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

Number::Number(const std::string &value, const location &loc)
  : Expr(loc) {
    try {
        this->value = int64_t(stoll(value, nullptr, 0));
    } catch (std::invalid_argument &e) {
        throw RumurError("invalid number: " + value, loc);
    } catch (std::out_of_range &e) {
        throw RumurError("out of range number: " + value, loc);
    }
}

Number::Number(int64_t value, const location &loc)
  : Expr(loc), value(value) {
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

}
