#include "generate.h"
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

void generate_property(std::ostream &out, const Property &p) {
  out << *p.expr;
}
