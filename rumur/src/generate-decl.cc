#include <cassert>
#include "generate.h"
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

void generate_decl(std::ostream &out, const Decl &d) {

  if (auto c = dynamic_cast<const ConstDecl*>(&d)) {
    assert((c->type == nullptr || c->type->is_simple())
      && "complex const decl");

    out << "static const value_t ru_" << c->name << " = VALUE_C("
      << c->value->constant_fold() << ")";

    return;
  }

  if (auto v = dynamic_cast<const VarDecl*>(&d)) {

    // If this has a valid offset, it's a state variable.
    if (v->offset >= 0) {
      out << "const struct handle ru_" << v->name
        << " = { .base = s->data, .offset = SIZE_C(" << v->offset
        << "), .width = SIZE_C(" << v->type->width() << ") }";

    // Otherwise we need to allocate backing memory for it.
    } else {
      out << "uint8_t _ru_" << v->name << "[BITS_TO_BYTES("
        << v->type->width() << ")] = { 0 };\n"
        << "  const struct handle ru_" << v->name << " = { .base = _ru_"
        << v->name << ", .offset = 0ul, .width = SIZE_C(" << v->type->width()
        << ") }";

    }

    return;
  }

  assert(!"unexpected TypeDecl in generate_decl");
}
