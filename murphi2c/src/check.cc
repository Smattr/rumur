#include <cstddef>
#include "check.h"
#include <cstdlib>
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Check : public ConstTraversal {

 public:
  bool ok = true;

  void visit_isundefined(const IsUndefined&) final {
    if (ok) {
      std::cerr << "isundefined expressions are not supported\n";
      ok = false;
    }
  }

};

}

bool check(const Node &n) {
  Check c;
  c.dispatch(n);
  return c.ok;
}
