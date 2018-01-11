#include <cassert>
#include <iostream>
#include "location.hh"
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Number.h>
#include <stdexcept>
#include <string>

using namespace std;

namespace rumur {

static_assert(sizeof(long long) >= sizeof(int64_t),
    "a 64-bit integer cannot be read with stoll");

Number::Number(const string &value, const location &loc, Indexer&)
  : Expr(loc) {
    try {
        this->value = int64_t(stoll(value, nullptr, 0));
    } catch (invalid_argument &e) {
        throw RumurError("invalid number: " + value, loc);
    } catch (out_of_range &e) {
        throw RumurError("out of range number: " + value, loc);
    }
}

Number::Number(int64_t value, const location &loc, Indexer&)
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

void Number::rvalue(ostream &out) const {
    out << "INT64_C(" << value << ")";
}

}
