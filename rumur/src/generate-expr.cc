#include <cassert>
#include <cstddef>
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <string>
#include "utils.h"

using namespace rumur;

namespace {

class Generator : public ConstExprTraversal {

 private:
  std::ostream *out;
  bool lvalue;

 public:
  Generator(std::ostream &o, bool lvalue_): out(&o), lvalue(lvalue_) { }

  // Make emitting an rvalue more concise below
  Generator &operator<<(const Expr &e) {
    Generator g(*out, false);
    g.dispatch(e);
    return *this;
  }

  Generator &operator<<(const std::string &s) {
    *out << s;
    return *this;
  }

  void visit(const Add &n) final {
    if (lvalue)
      invalid(n);
    *this << "add(s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit(const And &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " && " << *n.rhs << ")";
  }

  void visit(const Div &n) final {
    if (lvalue)
      invalid(n);
    *this << "divide(s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit(const Element &n) final {
    if (lvalue && !n.is_lvalue())
      invalid(n);

    // First, determine the width of the array's elements

    const TypeExpr *t1 = n.array->type();
    assert(t1 != nullptr && "array with invalid type");
    const TypeExpr *t2 = t1->resolve();
    assert(t2 != nullptr && "array with invalid type");

    auto a = dynamic_cast<const Array&>(*t2);
    mpz_class element_width = a.element_type->width();

    // Second, determine the minimum and maximum values of the array's index type

    const TypeExpr *t3 = a.index_type->resolve();
    assert(t3 != nullptr && "array with invalid index type");

    mpz_class min, max;
    if (auto r = dynamic_cast<const Range*>(t3)) {
      min = r->min->constant_fold();
      max = r->max->constant_fold();
    } else if (auto e = dynamic_cast<const Enum*>(t3)) {
      min = 0;
      max = e->count() - 1;
    } else if (auto s = dynamic_cast<const Scalarset*>(t3)) {
      min = 0;
      max = s->bound->constant_fold() - 1;
    } else {
      assert(false && "array with invalid index type");
    }

    if (!lvalue && a.element_type->is_simple()) {
      const std::string lb = a.element_type->lower_bound();
      const std::string ub = a.element_type->upper_bound();
      *out << "handle_read(s, " << lb << ", " << ub << ", ";
    }

    *out << "handle_index(s, SIZE_C(" << element_width << "), VALUE_C(" << min
      << "), VALUE_C(" << max << "), ";
    if (lvalue) {
      generate_lvalue(*out, *n.array);
    } else {
      generate_rvalue(*out, *n.array);
    }
    *this << ", " << *n.index << ")";

    if (!lvalue && a.element_type->is_simple())
      *out << ")";
  }

  void visit(const Eq &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " == " << *n.rhs << ")";
  }

  void visit(const Exists &n) final {
    if (lvalue)
      invalid(n);

    *out << "({ bool result = false; ";
    generate_quantifier_header(*out, n.quantifier);
    *this << "if (" << *n.expr << ") { result = true; break; }";
    generate_quantifier_footer(*out, n.quantifier);
    *out << " result; })";
  }

  void visit(const ExprID &n) final {
    if (n.value == nullptr)
      throw Error("symbol \"" + n.id + "\" in expression is unresolved", n.loc);

    if (lvalue && !n.is_lvalue())
      invalid(n);

    /* This is a reference to a const. Note, this also covers enum
     * members.
     */
    if (auto c = dynamic_cast<const ConstDecl*>(n.value.get())) {
      assert(!lvalue && "const appearing as an lvalue");
      *out << "VALUE_C(" << c->value->constant_fold() << ")";
      return;
    }

    // This is either a state variable, a local variable or an alias.
    if (isa<AliasDecl>(n.value) || isa<VarDecl>(n.value)) {

      const TypeExpr *t = n.type();
      assert((!n.is_lvalue() || t != nullptr) && "lvalue without a type");

      if (!lvalue && n.is_lvalue() && t->is_simple()) {
        const std::string lb = t->lower_bound();
        const std::string ub = t->upper_bound();
        *out << "handle_read(s, " << lb << ", " << ub << ", ";
      }

      *out << "ru_" << n.id;

      if (!lvalue && n.is_lvalue() && t->is_simple())
        *out << ")";
      return;
    }

    // FIXME: there's another case here where it's a reference to a quanitified
    // variable. I suspect we should just handle that the same way as a local.
  }

  void visit(const Field &n) final {
    if (lvalue && !n.is_lvalue())
      invalid(n);

    const TypeExpr *root = n.record->type();
    assert(root != nullptr);
    const TypeExpr *resolved = root->resolve();
    assert(resolved != nullptr);
    if (auto r = dynamic_cast<const Record*>(resolved)) {
      mpz_class offset = 0;
      for (const Ptr<VarDecl> &f : r->fields) {
        if (f->name == n.field) {
          if (!lvalue && f->type->is_simple()) {
            const std::string lb = f->type->lower_bound();
            const std::string ub = f->type->upper_bound();
            *out << "handle_read(s, " << lb << ", " << ub << ", ";
          }
          *out << "handle_narrow(";
          if (lvalue) {
            generate_lvalue(*out, *n.record);
          } else {
            generate_rvalue(*out, *n.record);
          }
          *out << ", " << offset << ", " << f->type->width() << ")";
          if (!lvalue && f->type->is_simple())
            *out << ")";
          return;
        }
        offset += f->type->width();
      }
      throw Error("no field named \"" + n.field + "\" in record", n.loc);
    }
    throw Error("left hand side of field expression is not a record", n.loc);
  }

  void visit(const Forall &n) final {
    if (lvalue)
      invalid(n);

    *out << "({ bool result = true; ";
    generate_quantifier_header(*out, n.quantifier);
    *this << "if (!" << *n.expr << ") { result = false; break; }";
    generate_quantifier_footer(*out, n.quantifier);
    *out << " result; })";
  }

  void visit(const FunctionCall &n) final {
    if (lvalue)
      invalid(n);

    if (n.function == nullptr)
      throw Error("unresolved function reference " + n.name, n.loc);

    const Ptr<TypeExpr> &return_type = n.function->return_type;

    *out << "ru_" << n.name << "(state_drop_const(s)";

    // Pass the return type output parameter if required.
    if (return_type != nullptr && !return_type->is_simple())
      *out << ", (struct handle){ .base = ret" << n.unique_id
        << ", .offset = 0ul, .width = SIZE_C(" << return_type->width() << ") }";

    // Now emit the arguments to the function.
    {
      auto it = n.function->parameters.begin();
      for (const Ptr<Expr> &a : n.arguments) {

        *out << ", ";

        assert(it != n.function->parameters.end() &&
          "function call has more arguments than its target function");

        const Ptr<VarDecl> &p = *it;

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

  void visit(const Geq &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " >= " << *n.rhs << ")";
  }

  void visit(const Gt &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " > " << *n.rhs << ")";
  }

  void visit(const Implication &n) final {
    if (lvalue)
      invalid(n);
    *this << "(!" << *n.lhs << " || " << *n.rhs << ")";
  }

  void visit(const Leq &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " <= " << *n.rhs << ")";
  }

  void visit(const Lt &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " < " << *n.rhs << ")";
  }

  void visit(const Mod &n) final {
    if (lvalue)
      invalid(n);
    *this << "mod(s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit(const Mul &n) final {
    if (lvalue)
      invalid(n);
    *this << "mul(s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit(const Negative &n) final {
    if (lvalue)
      invalid(n);
    *this << "negate(s, " << *n.rhs << ")";
  }

  void visit(const Neq &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " != " << *n.rhs << ")";
  }

  void visit(const Not &n) final {
    if (lvalue)
      invalid(n);
    *this << "(!" << *n.rhs << ")";
  }

  void visit(const Number &n) final {
    *out << "VALUE_C(" << n.value << ")";
  }

  void visit(const Or &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " || " << *n.rhs << ")";
  }
   
  void visit(const Sub &n) final {
    if (lvalue)
      invalid(n);
    *this << "sub(s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit(const Ternary &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.cond << " ? " << *n.lhs << " : " << *n.rhs << ")";
  }

  virtual ~Generator() = default;

 private:
  void invalid(const Expr &n) const {
    throw Error("invalid expression used as lvalue", n.loc);
  }
};

}

void generate_lvalue(std::ostream &out, const Expr &e) {
  Generator g(out, true);
  g.dispatch(e);
}

void generate_rvalue(std::ostream &out, const Expr &e) {
  Generator g(out, false);
  g.dispatch(e);
}
