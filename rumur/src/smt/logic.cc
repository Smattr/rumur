#include "logic.h"
#include "../options.h"
#include "except.h"
#include <cstddef>
#include <gmpxx.h>
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
    return "(_ bv" + value.get_str() + " " + std::to_string(BITVECTOR_WIDTH) +
           ")";
  }

  return value.get_str();
}

std::string add() {

  if (options.smt.use_bitvectors) {
    return "bvadd";
  }

  return "+";
}

std::string band() {

  if (options.smt.use_bitvectors) {
    return "bvand";
  }

  throw Unsupported(
      "SMT simplification involving bitwise AND is only "
      "supported when using a bitvector representation (--smt-bitvectors on)");
}

std::string bnot() {

  if (options.smt.use_bitvectors) {
    return "bvnot";
  }

  throw Unsupported(
      "SMT simplification involving bitwise NOT is only "
      "supported when using a bitvector representation (--smt-bitvectors on)");
}

std::string bor() {

  if (options.smt.use_bitvectors) {
    return "bvor";
  }

  throw Unsupported(
      "SMT simplification involving bitwise OR is only "
      "supported when using a bitvector representation (--smt-bitvectors on)");
}

std::string bxor() {

  if (options.smt.use_bitvectors) {
    return "bvxor";
  }

  throw Unsupported(
      "SMT simplification involving bitwise XOR is only "
      "supported when using a bitvector representation (--smt-bitvectors on)");
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

std::string gt() {

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

std::string lsh() {

  if (options.smt.use_bitvectors)
    return "bvshl";

  throw Unsupported(
      "SMT simplification involving left shifts is only "
      "supported when using a bitvector representation (--smt-bitvectors on)");
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

std::string rsh() {

  if (options.smt.use_bitvectors)
    return "bvashr";

  throw Unsupported(
      "SMT simplification involving right shifts is only "
      "supported when using a bitvector representation (--smt-bitvectors on)");
}

std::string sub() {

  if (options.smt.use_bitvectors) {
    return "bvsub";
  }

  return "-";
}

} // namespace smt
