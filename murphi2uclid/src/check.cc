#include "check.h"
#include <cstddef>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Checker : public ConstTraversal {

public:
  void visit_lsh(const Lsh &n) final {
    // TODO: technically we could implement this as a Uclid5 function. However,
    // it is a little awkward because Uclid5 does not support generic functions
    // so we would have to detect which types << is used with and emit a
    // function for each of these.
    throw Error("Uclid5 has no equivalent of the left shift operator", n.loc);
  }

  void visit_mod(const Mod &n) final {
    throw Error("Uclid5 has no equivalent of the modulo operator", n.loc);
  }

  void visit_rsh(const Rsh &n) final {
    // TODO: technically we could implement this as a Uclid5 function. However,
    // it is a little awkward because Uclid5 does not support generic functions
    // so we would have to detect which types >> is used with and emit a
    // function for each of these.
    throw Error("Uclid5 has no equivalent of the right shift operator", n.loc);
  }
};

} // namespace

void check(const Node &n) {
  Checker c;
  c.dispatch(n);
}
