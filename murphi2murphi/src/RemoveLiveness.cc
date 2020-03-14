#include <cstddef>
#include <cassert>
#include <ctype.h>
#include "RemoveLiveness.h"
#include <rumur/rumur.h>
#include "Stage.h"
#include <string>

using namespace rumur;

RemoveLiveness::RemoveLiveness(Stage &next_): IntermediateStage(next_) { }

void RemoveLiveness::write(const std::string &c) {

  // if we are not hunting a to-be-deleted semi-colon, we are done
  if (!swallow_semi) {
    next << c;
    return;
  }

  // if this is white space, remain on the hunt for a semi-colon
  if (c.size() == 1 && isspace(c.c_str()[0])) {
    next << c;
    return;
  }

  // if we reached here, we know one way or another we are done watching for
  // semi-colons
  swallow_semi = false;

  // if this is a semi-colon, suppress it
  if (c == ";")
    return;

  // otherwise, let it go through
  next << c;
}

void RemoveLiveness::visit_propertyrule(const PropertyRule &n) {

  // if this is not a liveness property, we can let it pass through
  if (n.property.category != Property::LIVENESS) {
    next.visit_propertyrule(n);
    return;
  }

  // seek to the start of this property
  top->sync_to(n);

  // omit the property itself
  top->skip_to(n.loc.end);

  // note that we may now encounter a semi-colon that needs to be deleted
  swallow_semi = true;
}
