#include "compares_complex_values.h"
#include <cstddef>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class ComplexComparisonFinder : public ConstTraversal {

private:
  bool seen = false;

public:
  void visit_eq(const Eq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);

    seen |= !n.lhs->type()->is_simple();

    // we do not need to look at the type of the RHS as we assume the model has
    // already been validated, thus all comparisons are type checked, and
    // therefore the type of the RHS is identical
  }

  void visit_neq(const Neq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);

    seen |= !n.lhs->type()->is_simple();
  }

  bool found() const { return seen; }
};

} // namespace

bool compares_complex_values(const Node &n) {
  ComplexComparisonFinder ccf;
  ccf.dispatch(n);
  return ccf.found();
}
