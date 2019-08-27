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
  const std::string handle;
  const bool support_diff;
  const bool support_xml;

 public:
  Generator(std::ostream &o, const std::string &p, const std::string &h,
    bool s, bool x):
    out(&o), prefix(p), handle(h), support_diff(s), support_xml(x) { }

  Generator(const Generator &caller, const std::string &p, const std::string &h):
    out(caller.out), prefix(p), handle(h), support_diff(caller.support_diff),
    support_xml(caller.support_xml) { }

  void visit_array(const Array &n) final {

    const Ptr<TypeExpr> t = n.index_type->resolve();

    if (auto r = dynamic_cast<const Range*>(t.get())) {

      mpz_class lb = r->min->constant_fold();
      mpz_class ub = r->max->constant_fold();

      // FIXME: Unrolling this loop at generation time is not great if the index
      // type is big. E.g. if someone has an 'array [0..10000] of ...' this is
      // going to generate quite unpleasant code.
      mpz_class preceding_offset = 0;
      mpz_class w = n.element_type->width();
      for (mpz_class i = lb; i <= ub; i++) {
        const std::string h = derive_handle(preceding_offset, w);
        Generator g(*this, prefix + "[" + i.get_str() + "]", h);
        g.dispatch(*n.element_type);
        preceding_offset += w;
      }

      return;
    }

    if (auto s = dynamic_cast<const Scalarset*>(t.get())) {

      mpz_class b = s->bound->constant_fold();

      mpz_class preceding_offset = 0;
      mpz_class w = n.element_type->width();
      for (mpz_class i = 0; i < b; i++) {
        const std::string h = derive_handle(preceding_offset, w);
        Generator g(*this, prefix + "[" + i.get_str() + "]", h);
        g.dispatch(*n.element_type);
        preceding_offset += w;
      }

      return;
    }

    if (auto e = dynamic_cast<const Enum*>(t.get())) {

      mpz_class preceding_offset = 0;
      mpz_class w = n.element_type->width();
      for (const std::pair<std::string, location> &m : e->members) {
        const std::string h = derive_handle(preceding_offset, w);
        Generator g(*this, prefix + "[" + m.first + "]", h);
        g.dispatch(*n.element_type);
        preceding_offset += w;
      }

      return;
    }

    assert(!"non-range, non-enum used as array index");
  }

  void visit_enum(const Enum &n) final {
    const std::string previous_handle = to_previous();

    *out
      << "{\n"
      << "  raw_value_t v = handle_read_raw(" << handle << ");\n"
      << "  raw_value_t v_previous = 0;\n";
    if (!support_diff)
      *out << "  const struct state *previous = NULL;\n";
    *out
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw(" << previous_handle << ");\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"%s\\\" value=\\\"\", \""
        << prefix << "\");\n"
      << "    } else {\n"
      << "      printf(\"%s:\", \"" << prefix << "\");\n"
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
      << "      assert(!\"illegal value for " << prefix << "\");\n"
      << "    }\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"\\\"/>\");\n"
      << "    }\n"
      << "    printf(\"\\n\");\n"
      << "  }\n"
      << "}\n";
  }

  void visit_range(const Range &n) final {

    const std::string lb = n.lower_bound();
    const std::string ub = n.upper_bound();

    const std::string previous_handle = to_previous();

    *out
      << "{\n"
      << "  raw_value_t v = handle_read_raw(" << handle << ");\n"
      << "  raw_value_t v_previous = 0;\n";
    if (!support_diff)
      *out << "  const struct state *previous = NULL;\n";
    *out
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw(" << previous_handle << ");\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"%s\\\" value=\\\"\", \""
        << prefix << "\");\n"
      << "    } else {\n"
      << "      printf(\"%s:\", \"" << prefix << "\");\n"
      << "    }\n"
      << "    if (v == 0) {\n"
      << "      printf(\"Undefined\");\n"
      << "    } else {\n"
      << "      printf(\"%\" PRIVAL, value_to_string(decode_value(" << lb << ", "
        << ub << ", v)));\n"
      << "    }\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"\\\"/>\");\n"
      << "    }\n"
      << "    printf(\"\\n\");\n"
      << "  }\n"
      << "}\n";
  }

  void visit_record(const Record &n) final {
    mpz_class preceding_offset = 0;
    for (auto &f : n.fields) {
      mpz_class w = f->width();
      const std::string h = derive_handle(preceding_offset, w);
      generate_print(*out, *f->type, prefix + "." + f->name, h, support_diff,
        support_xml);
      preceding_offset += w;
    }
  }

  void visit_scalarset(const Scalarset&) final {
    const std::string previous_handle = to_previous();

    *out
      << "{\n"
      << "  raw_value_t v = handle_read_raw(" << handle << ");\n"
      << "  raw_value_t v_previous = 0;\n";
    if (!support_diff)
      *out << "  const struct state *previous = NULL;\n";
    *out
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw(" << previous_handle << ");\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"%s\\\" value=\\\"\", \""
        << prefix << "\");\n"
      << "    } else {\n"
      << "      printf(\"%s:\", \"" << prefix << "\");\n"
      << "    }\n"
      << "    if (v == 0) {\n"
      << "      printf(\"Undefined\");\n"
      << "    } else {\n"
      << "      printf(\"%\" PRIVAL, value_to_string(v - 1));\n"
      << "    }\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"\\\"/>\");\n"
      << "    }\n"
      << "    printf(\"\\n\");\n"
      << "  }\n"
      << "}\n";
  }

  void visit_typeexprid(const TypeExprID &n) final {
    if (n.referent == nullptr)
      throw Error("unresolved type symbol \"" + n.name + "\"", n.loc);
    dispatch(*n.referent);
  }

  virtual ~Generator() = default;

 private:
  std::string derive_handle(mpz_class offset, mpz_class width) const {
    return "((struct handle){ .base = " + handle + ".base + (" + handle
      + ".offset + ((size_t)" + offset.get_str() + ")) / CHAR_BIT, .offset = ("
      + handle + ".offset + ((size_t)" + offset.get_str() + ")) % CHAR_BIT, "
      + ".width = " + width.get_str() + "ull })";
  }

  std::string to_previous() const {
    return "((struct handle){ .base = (uint8_t*)previous->data + (" + handle
      + ".base - (const uint8_t*)s->data), .offset = "
      + handle + ".offset, .width = " + handle + ".width })";
  }
};

}

void generate_print(std::ostream &out, const TypeExpr &e,
  const std::string &prefix, const std::string &handle, bool support_diff,
  bool support_xml) {

  Generator g(out, prefix, handle, support_diff, support_xml);
  g.dispatch(e);
}
