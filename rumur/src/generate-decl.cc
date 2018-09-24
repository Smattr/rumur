#include <cassert>
#include <cstddef>
#include "generate.h"
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

void generate_decl(std::ostream &out, const Decl &d) {

  if (auto a = dynamic_cast<const AliasDecl*>(&d)) {

    const TypeExpr *t = a->value->type();
    if ((t == nullptr || t->is_simple()) && !a->value->is_lvalue()) {
      out << "value_t";
    } else {
      out << "struct handle";
    }

    out << " " << a->name << " = ";
    if (a->value->is_lvalue()) {
      generate_lvalue(out, *a->value);
    } else {
      generate_rvalue(out, *a->value);
    }

    return;
  }

  if (auto c = dynamic_cast<const ConstDecl*>(&d)) {
    assert((c->type == nullptr || c->type->is_simple())
      && "complex const decl");

    out << "static const value_t ru_" << c->name << " __attribute__((unused)) "
      << "= VALUE_C(" << c->value->constant_fold()
      << ")";

    return;
  }

  if (auto v = dynamic_cast<const VarDecl*>(&d)) {

    // If this has a valid offset, it's a state variable.
    if (v->offset >= 0) {
      out << "const struct handle ru_" << v->name << " __attribute__((unused)) "
        << "= { .base = (uint8_t*)s->data, .offset = SIZE_C(" << v->offset
        << "), .width = SIZE_C(" << v->type->width() << ") }";

    // Otherwise we need to allocate backing memory for it.
    } else {
      out << "uint8_t _ru_" << v->name << "[BITS_TO_BYTES("
        << v->type->width() << ")] = { 0 };\n"
        << "  const struct handle ru_" << v->name << " __attribute__((unused)) "
        << "= { .base = _ru_" << v->name << ", .offset = 0ul, .width = SIZE_C("
        << v->type->width() << ") }";

    }

    return;
  }

  assert(!"unexpected TypeDecl in generate_decl");
}
