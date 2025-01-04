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
  bool need_word = false; ///< does this need a word type?

  // bitwise operations are only valid on word types
  void visit_band(const Band &n) final {
    need_word = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }

  void visit_bnot(const Bnot &n) final {
    need_word = true;
    n.rhs->visit(*this);
  }

  void visit_bor(const Bor &n) final {
    need_word = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }

  void visit_lsh(const Lsh &n) final {
    need_word = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }

  void visit_rsh(const Rsh &n) final {
    need_word = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }

  void visit_xor(const Xor &n) final {
    need_word = true;
    n.lhs->visit(*this);
    n.rhs->visit(*this);
  }
};

} // namespace

std::string pick_numeric_type(const Node &n) {

  Picker p;
  n.visit(p);

  if (p.need_word)
    return "word";

  // if there are no constraints, default to integer
  return "integer";
}
