#include <cassert>
#include <cstddef>
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
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

    for (const std::shared_ptr<VarDecl> &f : r->fields) {

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

    for (const std::shared_ptr<AliasDecl> &a : s.aliases) {
      *out << "    ";
      generate_decl(*out, *a);
      *out << ";\n";
    }

    for (const std::shared_ptr<Stmt> &st : s.body) {
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
    generate_quantifier_header(*out, *s.quantifier);
    for (const std::shared_ptr<Stmt> &st : s.body) {
      *out << "  ";
      generate_stmt(*out, *st);
      *out << ";\n";
    }
    generate_quantifier_footer(*out, *s.quantifier);
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
      for (const std::shared_ptr<Stmt> &st : c.body) {
        generate_stmt(*out, *st);
        *out << ";\n";
      }
      *out << "}\n";
      first = false;
    }
  }

  void visit(const ProcedureCall &s) final {
    if (s.function == nullptr)
      throw Error("unresolved procedure reference " + s.name, s.loc);

    *out << "ru_" << s.name << "(s";

    /* If the target of this call is actually a function (not a procedure) and
     * returns a complex type, we need to pass it a handle to the memory we've
     * allocated for its (unused) return.
     */
    const std::shared_ptr<TypeExpr> &return_type = s.function->return_type;
    if (return_type != nullptr && !return_type->is_simple())
      *out << ", (struct handle){ .base = ret" << s.unique_id
        << ", .offset = 0ul, .width = SIZE_C(" << return_type->width() << ") }";

    // Now emit the arguments to the procedure.
    {
      auto it = s.function->parameters.begin();
      for (const std::shared_ptr<Expr> &a : s.arguments) {

        *out << ", ";

        assert(it != s.function->parameters.end() &&
          "procedure call has more arguments than its target procedure");

        std::shared_ptr<VarDecl> &p = *it;

        if (!p->readonly) {
          generate_lvalue(*out, *a);
        } else {
          generate_rvalue(*out, *a);
        }

        it++;
      }
    }

    *out << ")";
  }

  void visit(const PropertyStmt &s) final {
    switch (s.property.category) {

      case Property::DISABLED:
        *out << "do { } while (0)";
        break;

      case Property::ASSERTION:
        *out << "if (__builtin_expect(!";
        generate_property(*out, s.property);
        *out << ", 0)) {\nerror(s, false, \"" << s.message << "\");\n}";
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

  void visit(const Undefine &s) final {
    *out << "handle_zero(";
    generate_lvalue(*out, *s.rhs);
    *out << ")";
  }

  virtual ~Generator() { }
};

}

void generate_stmt(std::ostream &out, const rumur::Stmt &s) {
  Generator g(out);
  g.dispatch(s);
}
