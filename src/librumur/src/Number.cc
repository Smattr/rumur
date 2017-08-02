#include <cassert>
#include "location.hh"
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Number.h>
#include <stdexcept>
#include <string>

using namespace rumur;
using namespace std;

static_assert(sizeof(long long) >= sizeof(int64_t),
    "a 64-bit integer cannot be read with stoll");

Number::Number(const string &value, const location &loc)
  : Expr(loc) {
    try {
        this->value = int64_t(stoll(value, nullptr, 0));
    } catch (invalid_argument &e) {
        throw RumurError("invalid number: " + value);
    } catch (out_of_range &e) {
        throw RumurError("out of range number: " + value);
    }
}

Number::Number(const Number &other, const location &loc) noexcept
  : Expr(loc), value(other.value) {
}
