#include "assume-statements-count.h"
#include <cstddef>
#include <rumur/rumur.h>

using namespace rumur;

unsigned long assume_statements_count(const Model &model) {

  // define a traversal for counting assume statements
  class AssumeCounter : public ConstTraversal {

  public:
    unsigned long count = 0;

    void visit_propertystmt(const PropertyStmt &n) final {
      if (n.property.category == Property::ASSUMPTION)
        count++;
      // no need to descend into children
    }

    virtual ~AssumeCounter() = default;
  };

  // use the counter to find how many assume statements we have
  AssumeCounter ac;
  ac.dispatch(model);

  return ac.count;
}
