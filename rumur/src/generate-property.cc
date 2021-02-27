#include "generate.h"
#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

void generate_property(std::ostream &out, const Property &p) {
  generate_rvalue(out, *p.expr);
}
