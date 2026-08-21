#include "check.h"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Check : public ConstTraversal {

public:
  bool ok = true;

  void visit_ismember(const IsMember &) final {
    if (ok) {
      std::cerr << "ismember expressions are not supported\n";
      ok = false;
    }
  }

  void visit_isundefined(const IsUndefined &) final {
    if (ok) {
      std::cerr << "isundefined expressions are not supported\n";
      ok = false;
    }
  }

  void visit_multiset(const Multiset &) final {
    if (ok) {
      std::cerr << "multiset types are not supported\n";
      ok = false;
    }
  }

  void visit_multisetadd(const MultisetAdd &) final {
    if (ok) {
      std::cerr << "multiset types are not supported\n";
      ok = false;
    }
  }

  void visit_multisetcount(const MultisetCount &) final {
    if (ok) {
      std::cerr << "multiset types are not supported\n";
      ok = false;
    }
  }

  void visit_union(const Union &) final {
    if (ok) {
      std::cerr << "union types are not supported\n";
      ok = false;
    }
  }
};

} // namespace

bool check(const Node &n) {
  Check c;
  c.dispatch(n);
  return c.ok;
}
