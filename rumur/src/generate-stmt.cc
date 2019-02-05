#include <cassert>
#include <cstddef>
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include "utils.h"

using namespace rumur;

static void clear(std::ostream &out, const rumur::TypeExpr &t,
    const std::string &offset = "SIZE_C(0)", size_t depth = 0) {

  const std::string indent = std::string(2 * (depth + 1), ' ');

  if (t.is_simple()) {
    out << indent << "handle_write_raw((struct handle){ .base = root.base, "
      << ".offset = root.offset + " << offset << ", .width = SIZE_C("
      << t.width() << ") }, 1);\n";

    return;
  }

  if (auto a = dynamic_cast<const Array*>(t.resolve())) {

    // The number of elements in this array as a C code string
    mpz_class ic = a->index_type->count() - 1;
    const std::string ub = "SIZE_C(" + ic.get_str() + ")";

    // The bit size of each array element as a C code string
    const std::string width = "SIZE_C(" +
      a->element_type->width().get_str() + ")";

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

  if (auto r = dynamic_cast<const Record*>(t.resolve())) {

    std::string off = offset;

    for (const Ptr<VarDecl> &f : r->fields) {

      // Generate code to clear this field
      clear(out, *f->type, off, depth);

      // Jump over this field to get the offset of the next field
      const std::string width = "SIZE_C(" + f->type->width().get_str() + ")";
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
  Generator(std::ostream &o): out(&o) { }

  void visit(const AliasStmt &s) final {
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
    
  void visit(const Assignment &s) final {

    if (s.lhs->type()->is_simple()) {
      const std::string lb = s.lhs->type()->lower_bound();
      const std::string ub = s.lhs->type()->upper_bound();

      *out << "handle_write(s, " << lb << ", " << ub << ", ";
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

  void visit(const Clear &s) final {
    *out
      << "do {\n"
      << "  struct handle root = ";
    generate_lvalue(*out, *s.rhs);
    *out << ";\n";

    assert(s.rhs->type() != nullptr && "clearing an expression without a type "
      "(non-lvalue?)");

    clear(*out, *s.rhs->type());

    *out << "} while (0)";
  }

  void visit(const ErrorStmt &s) final {
    *out << "error(s, false, \"" << s.message << "\")";
  }

  void visit(const For &s) final {
    generate_quantifier_header(*out, s.quantifier);
    for (auto &st : s.body) {
      *out << "  ";
      generate_stmt(*out, *st);
      *out << ";\n";
    }
    generate_quantifier_footer(*out, s.quantifier);
  }

  void visit(const If &s) final {
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
        if (auto e = dynamic_cast<const Eq*>(&*c.condition)) {
          if (e->lhs->type() != nullptr && !e->lhs->type()->is_simple())
            needs_bracketing = true;
        }
        if (auto e = dynamic_cast<const Neq*>(&*c.condition)) {
          if (e->lhs->type() != nullptr && !e->lhs->type()->is_simple())
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
          *out  << ") ";
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

  void visit(const ProcedureCall &s) final {
    generate_rvalue(*out, s.call);
  }

  void visit(const PropertyStmt &s) final {
    switch (s.property.category) {

      case Property::DISABLED:
        *out << "do { } while (0)";
        break;

      case Property::ASSERTION:
        *out << "if (__builtin_expect(!";
        generate_property(*out, s.property);
        *out << ", 0)) {\nerror(s, false, \"Assertion failed: %s\", \"";
        if (s.message == "") {
          /* Assertion has no associated text. Use the expression itself
           * instead.
           */
          *out << s.property.expr->to_string();
        } else {
          *out << s.message;
        }
        *out << "\");\n}";
        break;

      case Property::ASSUMPTION:
        *out << "if (__builtin_expect(!";
        generate_property(*out, s.property);
        *out
          << ", 0)) {\n"
          << "  assert(JMP_BUF_NEEDED && \"longjmping without a setup jmp_buf\");\n"
          << "  longjmp(checkpoint, 1);\n"
          << "}";
        break;

    }
  }

  void visit(const Put &s) final {

    if (s.expr == nullptr) {
      *out << "printf(\"%s\", \"" << s.value << "\")";
      return;
    }

    if (s.expr->is_lvalue()) {
      assert(s.expr->type() != nullptr && "lvalue expression has numeric "
        "literal type");

      // Construct a string containing a handle to this expression
      std::ostringstream buffer;
      generate_lvalue(buffer, *s.expr);

      generate_print(*out, *s.expr->type(), s.expr->to_string(), buffer.str(),
        false, false);

      return;
    }

    assert((s.expr->type() == nullptr || s.expr->type()->is_simple())
      && "complex non-lvalue in put statement");

    if (s.expr->type() != nullptr) {
      if (auto e = dynamic_cast<const Enum*>(s.expr->type()->resolve())) {
        *out
          << "{\n"
          << "  value_t v = ";
        generate_rvalue(*out, *s.expr);
        *out << ";\n";
        size_t i = 0;
        for (const std::pair<std::string, location> &m : e->members) {
          *out << "  ";
          if (i != 0)
            *out << "else ";
          *out << "if (v == " << i << ") {\n"
            << "    printf(\"%s\", \"" << m.first << "\");\n"
            << "  }\n";
          i++;
        }
        if (!e->members.empty())
          *out
            << "else {\n"
            << "  assert(\"illegal value read from enum expression\");\n"
            << "}\n";

        *out << "}";
        return;
      }
    }

    *out << "printf(\"%s\", value_to_string(";
    generate_rvalue(*out, *s.expr);
    *out << ").data)";
  }

  void visit(const Return &s) final {

    if (s.expr == nullptr) {
      *out << "return";

    } else {
      if (s.expr->type() == nullptr || s.expr->type()->is_simple()) {
        *out << "return ";
        generate_rvalue(*out, *s.expr);
      } else {
        /* The caller will have passed us a handle 'ret' to memory they have
         * allocated. Copy into it now.
         */
        *out
          << "do {\n"
          << "  handle_copy(ret, ";
        generate_rvalue(*out, *s.expr);
        *out << ");\n"
          << "  return ret;\n"
          << "} while (0)";
      }
    }
  }

  void visit(const Switch &s) final {
    *out
      << "do {\n"
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

  void visit(const Undefine &s) final {
    *out << "handle_zero(";
    generate_lvalue(*out, *s.rhs);
    *out << ")";
  }

  void visit(const While &s) final {
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

}

void generate_stmt(std::ostream &out, const rumur::Stmt &s) {
  Generator g(out);
  g.dispatch(s);
}
