#include "pick_numeric_type.h"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

namespace {

/// a visitor that infers any numeric type requirements
class Picker : public ConstTraversal {

public:
  bool needs_bv = false;      ///< does this need a bit-vector type?
  bool needs_integer = false; ///< does this need integer as the type?

  // bitwise operations are only valid on bit-vector types
  void visit_band(const Band &n) final {
    needs_bv = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }

  void visit_bnot(const Bnot &n) final {
    needs_bv = true;
    n.rhs->visit(*this);
  }

  void visit_bor(const Bor &n) final {
    needs_bv = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }

  // negative numbers require integers
  void visit_negative(const Negative &n) final {
    if (n.rhs->constant()) {
      if (n.rhs->constant_fold() > 0)
        needs_integer = true;
    }
    n.rhs->visit(*this);
  }

  void visit_number(const Number &n) final {
    if (n.value < 0)
      needs_integer = true;
  }

  void visit_xor(const Xor &n) final {
    needs_bv = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }
};

} // namespace

std::string pick_numeric_type(const Node &n) {

  Picker p;
  n.visit(p);

  if (p.needs_bv && p.needs_integer) {
    std::cerr << "Model has conflicting requirements for a numeric type. You "
                 "will need to select one explicitly with --numeric-type\n";
    exit(EXIT_FAILURE);
  }

  if (p.needs_bv)
    return "bv64"; // use 64-bit precision as a default

  if (p.needs_integer)
    return "integer";

  // if there are no constraints, default to integer
  return "integer";
}
