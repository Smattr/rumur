#include "check.h"
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Check : public ConstTraversal {

public:
  void visit_union(const Union &n) final {
    throw Error("union types are not supported", n.loc);
  }
};
} // namespace

void check(const rumur::Node &n) {
  Check c;
  c.dispatch(n);
}
