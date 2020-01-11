#include <cstddef>
#include <gmpxx.h>
#include "logic.h"
#include "../options.h"
#include <string>

namespace smt {

static const size_t BITVECTOR_WIDTH = 64;

std::string integer_type() {

  if (options.smt.use_bitvectors) {
    return "(_ BitVec " + std::to_string(BITVECTOR_WIDTH) + ")";
  }

  return "Int";
}

std::string numeric_literal(const mpz_class &value) {

  if (value < 0)
    return "(" + neg() + " " + numeric_literal(-value) + ")";

  if (options.smt.use_bitvectors) {
    return "(_ bv" + value.get_str() + " " + std::to_string(BITVECTOR_WIDTH)
      + ")";
  }

  return value.get_str();
}

std::string add() {

  if (options.smt.use_bitvectors) {
    return "bvadd";
  }

  return "+";
}

std::string div() {

  if (options.smt.use_bitvectors) {
    return "bvsdiv";
  }

  // XXX: may cause solvers like CVC4 to fail with an error. Not visible to the
  // the user unless passing --debug though, so left as-is for now.
  return "div";
}

std::string geq() {

  if (options.smt.use_bitvectors) {
    return "bvsge";
  }

  return ">=";
}

std::string gt(void) {

  if (options.smt.use_bitvectors) {
    return "bvsgt";
  }

  return ">";
}

std::string leq() {

  if (options.smt.use_bitvectors) {
    return "bvsle";
  }

  return "<=";
}

std::string lt() {

  if (options.smt.use_bitvectors) {
    return "bvslt";
  }

  return "<";
}

std::string mod() {

  if (options.smt.use_bitvectors) {
    return "bvsmod";
  }

  return "mod";
}

std::string mul() {

  if (options.smt.use_bitvectors) {
    return "bvmul";
  }

  return "*";
}

std::string neg() {

  if (options.smt.use_bitvectors) {
    return "bvneg";
  }

  return "-";
}

std::string sub() {

  if (options.smt.use_bitvectors) {
    return "bvsub";
  }

  return "-";
}

}
