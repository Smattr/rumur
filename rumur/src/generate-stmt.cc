#include "../../common/escape.h"
#include "../../common/isa.h"
#include "generate.h"
#include "options.h"
#include "utils.h"
#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <sstream>
#include <string>

using namespace rumur;

static void clear(std::ostream &out, const TypeExpr &t,
                  const std::string &offset = "((size_t)0)", size_t depth = 0) {

  const std::string indent = std::string(2 * (depth + 1), ' ');

  if (t.is_simple()) {
    out << indent << "handle_write_raw(s, (struct handle){ .base = root.base + "
        << "(root.offset + " << offset << ") / CHAR_BIT, .offset = "
        << "(root.offset + " << offset << ") % CHAR_BIT, .width = " << t.width()
        << "ull }, 1);\n";

    return;
  }

  const Ptr<TypeExpr> type = t.resolve();

  if (auto a = dynamic_cast<const Array *>(type.get())) {

    // The number of elements in this array as a C code string
    mpz_class ic = a->index_type->count() - 1;
    const std::string ub = "((size_t)" + ic.get_str() + "ull)";

    // The bit size of each array element as a C code string
    const std::string width =
        "((size_t)" + a->element_type->width().get_str() + "ull)";

    // Generate a loop to iterate over all the elements
    const std::string var = "i" + std::to_string(depth);
    out << indent << "for (size_t " << var << " = 0; " << var << " < " << ub
        << "; " << var << "++) {\n";

    // Generate code to clear each element
    const std::string off = offset + " + " + var + " * " + width;
    clear(out, *a->element_type, off, depth + 1);

    // Close the loop
    out << indent << "}\n";

    return;
  }

  if (auto r = dynamic_cast<const Record *>(type.get())) {

    std::string off = offset;

    for (const Ptr<VarDecl> &f : r->fields) {

      // Generate code to clear this field
      clear(out, *f->type, off, depth);

      // Jump over this field to get the offset of the next field
      const std::string width =
          "((size_t)" + f->type->width().get_str() + "ull)";
      off += " + " + width;
    }

    return;
  }

  assert(!"unreachable");
}

namespace {

class Generator : public ConstStmtTraversal {

private:
  std::ostream *out;

public:
  Generator(std::ostream &o) : out(&o) {}

  void visit_aliasstmt(const AliasStmt &s) final {
    *out << "  {\n";

    for (auto &a : s.aliases) {
      *out << "    ";
      generate_decl(*out, *a);
      *out << ";\n";
    }

    for (auto &st : s.body) {
      *out << "    ";
      generate_stmt(*out, *st);
      *out << ";\n";
    }

    *out << "  }\n";
  }

  void visit_assignment(const Assignment &s) final {

    if (s.lhs->type()->is_simple()) {
      const std::string lb = s.lhs->type()->lower_bound();
      const std::string ub = s.lhs->type()->upper_bound();

      *out << "handle_write(" << to_C_string(s.loc) << ", rule_name, "
           << to_C_string(*s.lhs) << ", s, " << lb << ", " << ub << ", ";
      generate_lvalue(*out, *s.lhs);
      *out << ", ";
      generate_rvalue(*out, *s.rhs);
      *out << ")";

    } else {
      *out << "handle_copy(";
      generate_lvalue(*out, *s.lhs);
      *out << ", ";
      generate_rvalue(*out, *s.rhs);
      *out << ")";
    }
  }

  void visit_clear(const Clear &s) final {
    *out << "do {\n"
         << "  struct handle root = ";
    generate_lvalue(*out, *s.rhs);
    *out << ";\n";

    clear(*out, *s.rhs->type());

    *out << "} while (0)";
  }

  void visit_errorstmt(const ErrorStmt &s) final {
    *out << "error(s, \"%s\", \"" << escape(s.message) << "\")";
  }

  void visit_for(const For &s) final {
    generate_quantifier_header(*out, s.quantifier);
    for (auto &st : s.body) {
      *out << "  ";
      generate_stmt(*out, *st);
      *out << ";\n";
    }
    generate_quantifier_footer(*out, s.quantifier);
  }

  void visit_if(const If &s) final {
    bool first = true;
    for (const IfClause &c : s.clauses) {

      /* HACK: Equality comparisons against by-value function/procedure
       * parameters of simple type can result in code generation like:
       *
       *   if ((ru_x == ...)) {
       *
       * On compilers with -Wparentheses-equality (e.g. Apple's Clang 10.0.0)
       * this generates a spurious warning. To avoid this, we suppress the
       * duplicate brackets for any comparison using a binary operator.
       */
      bool needs_bracketing = !isa<BinaryExpr>(c.condition);

      // equality comparison with complex types does not cause double bracketing
      if (c.condition != nullptr) {
        if (auto e = dynamic_cast<const Eq *>(&*c.condition)) {
          if (!e->lhs->type()->is_simple())
            needs_bracketing = true;
        }
        if (auto e = dynamic_cast<const Neq *>(&*c.condition)) {
          if (!e->lhs->type()->is_simple())
            needs_bracketing = true;
        }
      }

      if (!first)
        *out << "else ";
      if (c.condition != nullptr) {
        *out << "if ";
        if (needs_bracketing)
          *out << "(";
        generate_rvalue(*out, *c.condition);
        if (needs_bracketing)
          *out << ") ";
      }
      *out << " {\n";
      for (auto &st : c.body) {
        generate_stmt(*out, *st);
        *out << ";\n";
      }
      *out << "}\n";
      first = false;
    }
  }

  void visit_procedurecall(const ProcedureCall &s) final {
    generate_rvalue(*out, s.call);
  }

  void visit_propertystmt(const PropertyStmt &s) final {
    switch (s.property.category) {

    case Property::ASSERTION:
      *out << "if (__builtin_expect(!";
      generate_property(*out, s.property);
      *out << ", 0)) {\nerror(s, \"Assertion failed: %s:" << s.loc
           << ": %s\", \"" << escape(input_filename) << "\", ";
      if (s.message == "") {
        /* Assertion has no associated text. Use the expression itself
         * instead.
         */
        *out << to_C_string(*s.property.expr);
      } else {
        *out << "\"" << escape(s.message) << "\"";
      }
      *out << ");\n}";
      break;

    case Property::ASSUMPTION:
      *out << "if (__builtin_expect(!";
      generate_property(*out, s.property);
      *out << ", 0)) {\n"
           << "  assert(JMP_BUF_NEEDED && \"longjmping without a setup "
              "jmp_buf\");\n"
           << "  siglongjmp(checkpoint, 1);\n"
           << "}";
      break;

    case Property::COVER:
      *out << "if (";
      generate_property(*out, s.property);
      *out << ") {\n"
           << "  (void)__atomic_fetch_add(&covers[COVER_"
           << s.property.unique_id << "], 1, __ATOMIC_SEQ_CST);\n"
           << "}";
      break;

    case Property::LIVENESS:
      assert(s.property.category != Property::LIVENESS &&
             "liveness property "
             "illegally appearing in statement instead of at the top level");
      break;
    }
  }

  void visit_put(const Put &s) final {

    if (s.expr == nullptr) {
      *out << "put(\"" << s.value << "\")";
      return;
    }

    if (s.expr->is_lvalue()) {

      // Construct a string containing a handle to this expression
      std::ostringstream buffer;
      generate_lvalue(buffer, *s.expr);

      generate_print(*out, *s.expr->type(), s.expr->to_string(), buffer.str(),
                     false, false);

      return;
    }

    assert(s.expr->type()->is_simple() &&
           "complex non-lvalue in put statement");

    const Ptr<TypeExpr> type = s.expr->type()->resolve();
    if (auto e = dynamic_cast<const Enum *>(type.get())) {
      *out << "{\n"
           << "  value_t v = ";
      generate_rvalue(*out, *s.expr);
      *out << ";\n";
      size_t i = 0;
      for (const std::pair<std::string, location> &m : e->members) {
        *out << "  ";
        if (i != 0)
          *out << "else ";
        *out << "if (v == " << i << ") {\n"
             << "    put(\"" << m.first << "\");\n"
             << "  }\n";
        i++;
      }
      if (!e->members.empty())
        *out << "else {\n"
             << "  assert(\"illegal value read from enum expression\");\n"
             << "}\n";

      *out << "}";
      return;
    }

    *out << "put_val(";
    generate_rvalue(*out, *s.expr);
    *out << ")";
  }

  void visit_return(const Return &s) final {

    if (s.expr == nullptr) {
      *out << "return true";

    } else {
      if (s.expr->type()->is_simple()) {
        *out << "return ";
        generate_rvalue(*out, *s.expr);
      } else {
        /* The caller will have passed us a handle 'ret' to memory they have
         * allocated. Copy into it now.
         */
        *out << "do {\n"
             << "  handle_copy(ret, ";
        generate_rvalue(*out, *s.expr);
        *out << ");\n"
             << "  return ret;\n"
             << "} while (0)";
      }
    }
  }

  void visit_switch(const Switch &s) final {
    *out << "do {\n"
         // Mark the following unused in case there are no cases
         << "  value_t v __attribute__((unused)) = ";
    generate_rvalue(*out, *s.expr);
    *out << ";\n";

    bool first_case = true;
    for (const SwitchCase &c : s.cases) {

      *out << "  ";
      if (!first_case)
        *out << "else ";
      if (!c.matches.empty()) {
        *out << "if (";
        bool first_match = true;
        for (auto &m : c.matches) {
          if (!first_match)
            *out << " || ";
          *out << "v == ";
          generate_rvalue(*out, *m);
          first_match = false;
        }
        *out << ") ";
      }
      *out << "{\n";

      for (auto &st : c.body) {
        *out << "    ";
        dispatch(*st);
        *out << ";\n";
      }

      *out << "  }\n";
      first_case = false;
    }

    *out << "} while (0)";
  }

  void visit_undefine(const Undefine &s) final {
    *out << "handle_zero(";
    generate_lvalue(*out, *s.rhs);
    *out << ")";
  }

  void visit_while(const While &s) final {
    *out << "while (";
    generate_rvalue(*out, *s.condition);
    *out << ") {\n";
    for (auto &st : s.body) {
      *out << "  ";
      generate_stmt(*out, *st);
      *out << ";\n";
    }
    *out << "}";
  }

  virtual ~Generator() = default;
};

} // namespace

void generate_stmt(std::ostream &out, const Stmt &s) {
  Generator g(out);
  g.dispatch(s);
}
