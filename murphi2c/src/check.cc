#include <cstddef>
#include "check.h"
#include <cstdlib>
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Check : public ConstTraversal {

 public:
  void visit_isundefined(const IsUndefined&) final {
    std::cerr << "isundefined expressions are not supported\n";
    exit(EXIT_FAILURE);
  }

};

}

void check(const Node &n) {
  Check c;
  c.dispatch(n);
}
