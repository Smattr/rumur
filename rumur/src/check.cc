#include "check.h"
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Check : public ConstTraversal {

public:
  void visit_choose(const Choose &n) final {
    throw Error{"choose rules are not supported", n.loc};
  }

  void visit_ismember(const IsMember &n) final {
    throw Error("ismember expressions are not supported", n.loc);
  }

  void visit_multiset(const Multiset &n) final {
    throw Error("multiset types are not supported", n.loc);
  }

  void visit_multisetadd(const MultisetAdd &n) final {
    throw Error("multiset types are not supported", n.loc);
  }

  void visit_multisetcount(const MultisetCount &n) final {
    throw Error("multiset types are not supported", n.loc);
  }

  void visit_multisetremove(const MultisetRemove &n) final {
    throw Error{"multiset types are not supported", n.loc};
  }

  void visit_multisetremovepred(const MultisetRemovePred &n) final {
    throw Error{"multiset types are not supported", n.loc};
  }

  void visit_union(const Union &n) final {
    throw Error("union types are not supported", n.loc);
  }
};
} // namespace

void check(const rumur::Node &n) {
  Check c;
  c.dispatch(n);
}
