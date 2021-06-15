#include "utils.h"
#include "../../common/escape.h"
#include "options.h"
#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <rumur/rumur.h>
#include <sstream>
#include <string>

using namespace rumur;

std::string to_C_string(const Expr &expr) {
  return "\"" + escape(expr.to_string()) + "\"";
}

static std::string to_string(const location &location) {
  std::stringstream ss;
  ss << location;
  return ss.str();
}

std::string to_C_string(const location &location) {
  return "\"" + escape(input_filename) + ":" + to_string(location) + ": \"";
}

mpz_class bit_width(const mpz_class &v) {

  // FIXME: this replicates TypeExpr::width(). It would be nice to consolidate
  // this functionality somewhere.

  assert(v >= 0);

  // if there are 0 or 1 values, the width is trivial
  if (v <= 1)
    return 0;

  /* otherwise, we need the number of bits required to represent the largest
   * value
   */
  mpz_class largest(v - 1);
  mpz_class bits(0);
  while (largest != 0) {
    bits++;
    largest >>= 1;
  }
  return bits;
}
