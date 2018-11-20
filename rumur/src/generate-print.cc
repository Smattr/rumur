#include <cassert>
#include <cstddef>
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <utility>

using namespace rumur;

namespace {

class Generator : public ConstTypeTraversal {

 private:
  std::ostream *out;
  const std::string prefix;
  mpz_class preceding_offset;

 public:
  Generator(std::ostream &o, const std::string &p, mpz_class po):
    out(&o), prefix(p), preceding_offset(po) { }

  void visit(const Array &n) final {

    const TypeExpr *t = n.index_type->resolve();

    if (auto r = dynamic_cast<const Range*>(t)) {

      mpz_class lb = r->min->constant_fold();
      mpz_class ub = r->max->constant_fold();

      // FIXME: Unrolling this loop at generation time is not great if the index
      // type is big. E.g. if someone has an 'array [0..10000] of ...' this is
      // going to generate quite unpleasant code.
      for (mpz_class i = lb; i <= ub; i++) {
        Generator g(*out, prefix + "[" + i.get_str() + "]", preceding_offset);
        g.dispatch(*n.element_type);
        preceding_offset += n.element_type->width();
      }

      return;
    }

    if (auto s = dynamic_cast<const Scalarset*>(t)) {

      mpz_class b = s->bound->constant_fold();

      for (mpz_class i = 0; i < b; i++) {
        Generator g(*out, prefix + "[" + i.get_str() + "]", preceding_offset);
        g.dispatch(*n.element_type);
        preceding_offset += n.element_type->width();
      }

      return;
    }

    if (auto e = dynamic_cast<const Enum*>(t)) {

      for (const std::pair<std::string, location> &m : e->members) {
        Generator g(*out, prefix + "[" + m.first + "]", preceding_offset);
        g.dispatch(*n.element_type);
        preceding_offset += n.element_type->width();
      }

      return;
    }

    assert(!"non-range, non-enum used as array index");
  }

  void visit(const Enum &n) final {
    *out
      << "{\n"
      << "  value_t v = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = SIZE_C(" << preceding_offset
        << "), .width = SIZE_C(" << n.width() << ") });\n"
      << "  value_t v_previous = VALUE_C(0);\n"
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)previous->data, .offset = SIZE_C(" << preceding_offset
        << "), .width = SIZE_C(" << n.width() << ") });\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"" << prefix.c_str() << "\\\" "
        << "value=\\\"\");\n"
      << "    } else {\n"
      << "      printf(\"" << prefix << ":\");\n"
      << "    }\n"
      << "    if (v == 0) {\n"
      << "      printf(\"Undefined\");\n";
    size_t i = 0;
    for (const std::pair<std::string, location> &m : n.members) {
      *out
        << "    } else if (v == VALUE_C(" << (i + 1) << ")) {\n"
        << "      printf(\"%s\", \"" << m.first << "\");\n";
      i++;
    }
    *out
      << "    } else {\n"
      << "      printf(\"ILLEGAL VALUE\");\n"
      << "    }\n"
      << "    if (MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"\\\"/>\");\n"
      << "    }\n"
      << "    printf(\"\\n\");\n"
      << "  }\n"
      << "}\n";
  }

  void visit(const Range &n) final {

    const std::string lb = n.lower_bound();
    const std::string ub = n.upper_bound();

    *out
      << "{\n"
      << "  value_t v = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = SIZE_C(" << preceding_offset
        << "), .width = SIZE_C(" << n.width() << ") });\n"
      << "  value_t v_previous = VALUE_C(0);\n"
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)previous->data, .offset = SIZE_C(" << preceding_offset
        << "), .width = SIZE_C(" << n.width() << ") });\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"" << prefix.c_str() << "\\\" "
        << "value=\\\"\");\n"
      << "    } else {\n"
      << "      printf(\"" << prefix << ":\");\n"
      << "    }\n"
      << "    if (v == 0) {\n"
      << "      printf(\"Undefined\");\n"
      << "    } else {\n"
      << "      printf(\"%\" PRIVAL \"\", decode_value(" << lb << ", "
        << ub << ", v));\n"
      << "    }\n"
      << "    if (MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"\\\"/>\");\n"
      << "    }\n"
      << "    printf(\"\\n\");\n"
      << "  }\n"
      << "}\n";
  }

  void visit(const Record &n) final {
    for (auto &f : n.fields) {
      generate_print(*out, *f, prefix + ".", preceding_offset);
      preceding_offset += f->width();
    }
  }

  void visit(const Scalarset &n) final {
    *out
      << "{\n"
      << "  value_t v = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = SIZE_C(" << preceding_offset
        << "), .width = SIZE_C(" << n.width() << ") });\n"
      << "  value_t v_previous = VALUE_C(0);\n"
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)previous->data, .offset = SIZE_C(" << preceding_offset
        << "), .width = SIZE_C(" << n.width() << ") });\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"" << prefix.c_str() << "\\\" "
        << "value=\\\"\");\n"
      << "    } else {\n"
      << "      printf(\"" << prefix << ":\");\n"
      << "    }\n"
      << "    if (v == 0) {\n"
      << "      printf(\"Undefined\");\n"
      << "    } else {\n"
      << "      printf(\"%\" PRIVAL \"\", v - 1);\n"
      << "    }\n"
      << "    if (MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"\\\"/>\");\n"
      << "    }\n"
      << "    printf(\"\\n\");\n"
      << "  }\n"
      << "}\n";
  }

  void visit(const TypeExprID &n) final {
    if (n.referent == nullptr)
      throw Error("unresolved type symbol \"" + n.name + "\"", n.loc);
    dispatch(*n.referent);
  }

  virtual ~Generator() = default;
};

}

void generate_print(std::ostream &out, const rumur::VarDecl &d,
  const std::string &prefix, mpz_class preceding_offset) {

  Generator g(out, prefix + d.name, preceding_offset);
  g.dispatch(*d.type);
}
