#include "find-liveness.h"
#include <rumur/rumur.h>
#include <vector>

using namespace rumur;

namespace {

class LivenessFinder : public ConstTraversal {

 public:
  // ranges of found liveness properties
  std::vector<location> ranges;

  void visit_propertyrule(const PropertyRule &n) final {
    if (n.property.category == Property::LIVENESS) {
      ranges.push_back(n.loc);
    }
  }

  void visit_propertystmt(const PropertyStmt &n) final {
    if (n.property.category == Property::LIVENESS) {
      ranges.push_back(n.loc);
    }
  }
};

}

std::vector<location> find_liveness(const Model &m) {
  LivenessFinder lf;
  lf.dispatch(m);
  return lf.ranges;
}
