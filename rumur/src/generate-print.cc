#include <cassert>
#include <cstddef>
#include "../../common/escape.h"
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;

namespace {

// dynamically constructed printf call
class Printf {

 private:
  std::vector<std::string> format;
  std::vector<std::string> parameters;

 public:
  Printf() { }

  explicit Printf(const std::string &s) {
    add_str(s);
  }

  void add_str(const std::string &s) {
    format.push_back("%s");
    parameters.push_back("\"" + escape(s) + "\"");
  }

  void add_val(const std::string &s) {
    format.push_back("%\" PRIVAL \"");
    parameters.push_back("value_to_string(" + s + ")");
  }

  Printf &operator<<(const std::string &s) {
    add_str(s);
    return *this;
  }

  // construct the final printf call
  std::string str() const {
    std::ostringstream r;
    r << "printf(\"";
    for (const std::string &f : format)
      r << f;
    r << "\"";
    for (const std::string &p : parameters)
      r << ", " << p;
    r << ")";
    return r.str();
  }
};

class Generator : public ConstTypeTraversal {

 private:
  std::ostream *out;
  const Printf prefix;
  const std::string handle;

  // generated handle to our target in the previous state
  const std::string previous_handle;

  const bool support_diff;
  const bool support_xml;

  // a counter used for creating unique symbols
  mpz_class var_counter = 0;

  // declaration for reading the schedule of the scalarset we are printing
  const TypeDecl *schedule_type = nullptr;

 public:
  Generator(std::ostream &o, const Printf &p, const std::string &h,
    const std::string &ph, bool s, bool x):
    out(&o), prefix(p), handle(h), previous_handle(ph), support_diff(s),
    support_xml(x) { }

  Generator(const Generator &caller, const Printf &p, const std::string &h,
    const std::string &ph):
    out(caller.out), prefix(p), handle(h), previous_handle(ph),
    support_diff(caller.support_diff), support_xml(caller.support_xml),
    var_counter(caller.var_counter) { }

  void visit_array(const Array &n) final {

    const Ptr<TypeExpr> t = n.index_type->resolve();

    if (auto r = dynamic_cast<const Range*>(t.get())) {

      const mpz_class lb = r->min->constant_fold();
      const mpz_class ub = r->max->constant_fold();
      const mpz_class bound = ub - lb + 1;

      // invent a loop counter
      const std::string i = "i" + var_counter.get_str();
      ++var_counter;

      // generate a loop that spans the index type
      *out
        << "{\n"
        << "  for (size_t " << i << " = 0; " << i << " < " << bound.get_str()
          << "ull; ++" << i << ") {\n";

      // construct a textual description of the current element
      Printf p = prefix;
      p << "[";
      p.add_val(
        "(raw_value_t)" + i + " + (raw_value_t)VALUE_C(" + lb.get_str() + ")");
      p << "]";

      // construct a dynamic handle to the current element
      mpz_class w = n.element_type->width();
      const std::string o = "(" + i + " * ((size_t)" + w.get_str() + "ull))";
      const std::string h = derive_handle(o, w);
      const std::string ph = to_previous(h);

      // generate the body of the loop (printing of the current element)
      Generator g(*this, p, h, ph);
      g.dispatch(*n.element_type);

      // close the loop
      *out
        << "  }\n"
        << "}\n";

      return;
    }

    if (auto s = dynamic_cast<const Scalarset*>(t.get())) {

      // figure out if this is a named scalarset (i.e. one eligible for symmetry
      // reduction)
      auto id = dynamic_cast<const TypeExprID*>(n.index_type.get());

      if (id != nullptr) {
        // remove any indirection (TypeExprID of a TypeExprID)
        while (auto inner = dynamic_cast<const TypeExprID*>(id->referent->value.get()))
          id = inner;
      }

      // invent a symbol we can use for the retrieved schedule
      const std::string sch = "schedule" + var_counter.get_str();
      ++var_counter;

      const mpz_class b = s->bound->constant_fold();

      if (id != nullptr) {
        // generate schedule retrieval
        *out
          << "{\n"
          << "  size_t " << sch << "[" << b.get_str() << "ull];\n"
          << "  /* setup a default identity mapping for when symmetry\n"
          << "   * reduction is off\n"
          << "   */\n"
          << "  for (size_t i = 0; i < " << b.get_str() << "ull; ++i) {\n"
          << "    " << sch << "[i] = i;\n"
          << "  }\n"
          << "  if (SYMMETRY_REDUCTION != SYMMETRY_REDUCTION_OFF) {\n"
          << "    size_t index = schedule_read_" << id->name << "(s);\n"
          << "    size_t stack[" << b.get_str() << "ull];\n"
          << "    index_to_permutation(index, " << sch << ", stack, (size_t)"
            << b.get_str() << "ull);\n"
          << "  }\n";
      }

      // invent a loop counter
      const std::string i = "i" + var_counter.get_str();
      ++var_counter;

      // generate a loop that spans the index type
      *out
        << "{\n"
        << "  for (size_t " << i << " = 0; " << i << " < " << b.get_str()
          << "ull; ++" << i << ") {\n";

      // invent a variable for the permuted value of the counter
      const std::string j = "j" + var_counter.get_str();
      ++var_counter;

      // determine permuted index of the current element
      *out << "    size_t " << j << ";\n";
      if (id != nullptr) {
        *out
          << "    for (" << j << " = 0; " << j << " < " << b.get_str()
            << "ull; ++" << j << ") {\n"
          << "      if (" << sch << "[" << j << "] == " << i << ") {\n"
          << "        break;\n"
          << "      }\n"
          << "    }\n"
          << "    assert(" << j << " < " << b.get_str() << "ull &&\n"
          << "      \"failed to find permuted scalarset index\");\n";
      } else {
        *out
          << "    " << j << " = " << i << ";\n";
      }

      // construct a textual description of the current element
      Printf p = prefix;
      p << "[";
      if (id != nullptr)
        p << id->name << "_";
      p.add_val(i);
      p << "]";

      // construct a dynamic handle to the current element
      mpz_class w = n.element_type->width();
      const std::string o = "(" + j + " * ((size_t)" + w.get_str() + "ull))";
      const std::string h = derive_handle(o, w);
      const std::string ph = to_previous(h);

      // generate the body of the loop (printing of the current element)
      Generator g(*this, p, h, ph);
      g.dispatch(*n.element_type);

      // close the loop
      *out
        << "  }\n"
        << "}\n";

      // close the scope opened for schedule retrieval
      if (id != nullptr)
        *out << "}\n";

      return;
    }

    if (auto e = dynamic_cast<const Enum*>(t.get())) {

      mpz_class preceding_offset = 0;
      mpz_class w = n.element_type->width();
      for (const std::pair<std::string, location> &m : e->members) {
        Printf p = prefix;
        p << "[" << m.first << "]";
        const std::string h = derive_handle(preceding_offset, w);
        const std::string ph = to_previous(h);
        Generator g(*this, p, h, ph);
        g.dispatch(*n.element_type);
        preceding_offset += w;
      }

      return;
    }

    assert(!"non-range, non-enum used as array index");
  }

  void visit_enum(const Enum &n) final {

    *out
      << "{\n"
      << "  raw_value_t v = handle_read_raw(s, " << handle << ");\n"
      << "  raw_value_t v_previous = 0;\n";
    if (!support_diff)
      *out << "  const struct state *previous = NULL;\n";
    *out
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw(previous, " << previous_handle << ");\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"\");\n"
      << "      " << prefix.str() << ";\n"
      << "      printf(\"\\\" value=\\\"\");\n"
      << "    } else {\n"
      << "      " << prefix.str() << ";\n"
      << "      printf(\":\");\n"
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
      << "      assert(!\"illegal value for enum\");\n"
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

    *out
      << "{\n"
      << "  raw_value_t v = handle_read_raw(s, " << handle << ");\n"
      << "  raw_value_t v_previous = 0;\n";
    if (!support_diff)
      *out << "  const struct state *previous = NULL;\n";
    *out
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw(previous, " << previous_handle << ");\n"
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"\");\n"
      << "      " << prefix.str() << ";\n"
      << "      printf(\"\\\" value=\\\"\");\n"
      << "    } else {\n"
      << "      " << prefix.str() << ";\n"
      << "      printf(\":\");\n"
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
      Printf p = prefix;
      p << "." << f->name;
      const std::string h = derive_handle(preceding_offset, w);
      const std::string ph = to_previous(h);
      Generator g(*this, p, h, ph);
      g.dispatch(*f->type);
      preceding_offset += w;
    }
  }

  void visit_scalarset(const Scalarset &n) final {

    const std::string bound
      = "((size_t)" + n.bound->constant_fold().get_str() + "ull)";

    *out
      << "{\n"
      << "  raw_value_t v = handle_read_raw(s, " << handle << ");\n"
      << "  raw_value_t v_previous = 0;\n";
    if (!support_diff)
      *out << "  const struct state *previous = NULL;\n";
    *out
      << "  if (previous != NULL) {\n"
      << "    v_previous = handle_read_raw(previous, " << previous_handle << ");\n";

    // did we identify the schedule mapping for this type?
    if (schedule_type != nullptr) {
      *out
        << "    if (SYMMETRY_REDUCTION != SYMMETRY_REDUCTION_OFF && v_previous != 0) {\n"
        << "      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {\n"
        << "        assert(PRINTS_SCALARSETS && \"accessing a scalarset \"\n"
        << "          \"schedule which was unanticipated; bug in\"\n"
        << "          \"prints_scalarsets()?\");\n"
        << "      }\n"
        << "      /* we can use the saved schedule to map this value back to a\n"
        << "       * more intuitive string for the user\n"
        << "       */\n"
        << "      size_t index = schedule_read_" << schedule_type->name << "(previous);\n"
        << "      size_t schedule[" << bound << "];\n"
        << "      size_t stack[" << bound << "];\n"
        << "      index_to_permutation(index, schedule, stack, " << bound << ");\n"
        << "      ASSERT((size_t)v_previous - 1 < " << bound << " && \"illegal scalarset \"\n"
        << "        \"value found during printing\");\n"
        << "      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];\n"
        << "    }\n";
    }

    *out
      << "  }\n"
      << "  if (previous == NULL || v != v_previous) {\n"
      << "    if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
      << "      printf(\"<state_component name=\\\"\");\n"
      << "      " << prefix.str() << ";\n"
      << "      printf(\"\\\" value=\\\"\");\n"
      << "    } else {\n"
      << "      " << prefix.str() << ";\n"
      << "      printf(\":\");\n"
      << "    }\n"
      << "    if (v == 0) {\n"
      << "      printf(\"Undefined\");\n";

    // did we identify the schedule mapping for this type?
    if (schedule_type != nullptr) {
      *out
        << "    } else if (SYMMETRY_REDUCTION != SYMMETRY_REDUCTION_OFF) {\n"
        << "      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {\n"
        << "        assert(PRINTS_SCALARSETS && \"accessing a scalarset \"\n"
        << "          \"schedule which was unanticipated; bug in\"\n"
        << "          \"prints_scalarsets()?\");\n"
        << "      }\n"
        << "      /* we can use the saved schedule to map this value back to a\n"
        << "       * more intuitive string for the user\n"
        << "       */\n"
        << "      size_t index = schedule_read_" << schedule_type->name << "(s);\n"
        << "      size_t schedule[" << bound << "];\n"
        << "      size_t stack[" << bound << "];\n"
        << "      index_to_permutation(index, schedule, stack, " << bound << ");\n"
        << "      if (" << support_xml << " && MACHINE_READABLE_OUTPUT) {\n"
        << "        xml_printf(\"" << escape(schedule_type->name) << "\");\n"
        << "      } else {\n"
        << "        printf(\"%s\", \"" << escape(schedule_type->name) << "\");\n"
        << "      }\n"
        << "      ASSERT((size_t)v - 1 < " << bound << " && \"illegal scalarset \"\n"
        << "        \"value found during printing\");\n"
        << "      printf(\"_%zu\", schedule[(size_t)v - 1]);\n";
    }

    *out
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

    // is this type a reference to a (symmetry reduced) scalarset?
    auto s = dynamic_cast<const Scalarset*>(n.referent->value.get());
    if (s != nullptr) {
      // save this declaration for later reading the schedule of this type
      schedule_type = n.referent.get();
    }

    dispatch(*n.referent->value);
  }

  virtual ~Generator() = default;

 private:
  std::string derive_handle(mpz_class offset, mpz_class width) const {
    return derive_handle("((size_t)" + offset.get_str() + ")", width);
  }

  std::string derive_handle(const std::string &offset, mpz_class width) const {
    return "((struct handle){ .base = " + handle + ".base + (" + handle
      + ".offset + " + offset + ") / CHAR_BIT, .offset = ("
      + handle + ".offset + " + offset + ") % CHAR_BIT, "
      + ".width = " + width.get_str() + "ull })";
  }

  std::string to_previous(const std::string &h) const {
    return "((struct handle){ .base = (uint8_t*)previous->data + (" + h
      + ".base - (const uint8_t*)s->data), .offset = "
      + h + ".offset, .width = " + h + ".width })";
  }
};

}

void generate_print(std::ostream &out, const TypeExpr &e,
  const std::string &prefix, const std::string &handle, bool support_diff,
  bool support_xml) {

  // construct an equivalent handle to this data in the previous state
  const std::string ph = "((struct handle){ .base = (uint8_t*)previous->data "
    "+ (" + handle + ".base - (const uint8_t*)s->data), .offset = " + handle +
    ".offset, .width = " + handle + ".width })";

  Generator g(out, Printf(prefix), handle, ph, support_diff, support_xml);
  g.dispatch(e);
}
