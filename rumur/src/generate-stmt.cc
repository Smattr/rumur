#include <cassert>
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

namespace {

class Generator : public ConstStmtTraversal {
 
 private:
  std::ostream *out;

 public:
  Generator(std::ostream &o): out(&o) { }
    
  void visit(const Assignment &s) final {

    if (!s.lhs->type()->is_simple())
      assert(!"TODO");

    const std::string lb = s.lhs->type()->lower_bound();
    const std::string ub = s.lhs->type()->upper_bound();

    *out << "handle_write(s, " << lb << ", " << ub << ", ";
    generate_lvalue(*out, *s.lhs);
    *out << ", ";
    generate_rvalue(*out, *s.rhs);
    *out << ")";
  }

  void visit(const Clear&) final {
    assert(!"TODO");
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
      if (!first)
        *out << "else ";
      if (c.condition != nullptr) {
        *out << "if (";
        generate_rvalue(*out, *c.condition);
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

    /* Open a scope in which to declare any complex types we need to pass by
     * value.
     */
    *out << "do {\n";

    // Allocate memory for each parameter of complex type passed by value.
    {
      size_t i = 0;
      auto it = s.function->parameters.begin();
      for (const std::shared_ptr<Expr> &a : s.arguments) {

        assert(it != s.function->parameters.end() &&
          "procedure call has more arguments than its target procedure");

        std::shared_ptr<Parameter> &p = *it;

        assert(p->decl->type != nullptr && "procedure parameter without a type");

        if (!p->by_reference && !p->decl->type->is_simple()) {
          mpz_class width = p->decl->type->width();
          *out
            << "  uint8_t param" << i << "[BITS_TO_BYTES(" << width << "];\n"
            << "  handle_copy((struct handle){ .base = param" << i
              << ", .offset = 0ul, .width = SIZE_C(" << width << ") }, ";
          generate_rvalue(*out, *a);
          *out << ");\n";
        }

        i++;
        it++;
      }
    }

    *out << "ru_" << s.name << "(s";

    // Now emit the arguments to the procedure.
    {
      size_t i = 0;
      auto it = s.function->parameters.begin();
      for (const std::shared_ptr<Expr> &a : s.arguments) {

        *out << ", ";

        std::shared_ptr<Parameter> &p = *it;

        if (p->by_reference) {
          generate_lvalue(*out, *a);
        } else if (p->decl->type->is_simple()) {
          generate_rvalue(*out, *a);
        } else {
          mpz_class width = p->decl->type->width();
          *out << "(struct handle){ .base = param" << i
            << ", .offset = 0ul, .width = SIZE_C(" << width << ") }";
        }

        i++;
        it++;
      }
    }

    *out << ");\n"
      << "} while (0)";
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
        assert(!"TODO");
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
