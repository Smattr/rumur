#include "../../common/isa.h"
#include "generate.h"
#include "utils.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

namespace {

class Generator : public ConstExprTraversal {

private:
  std::ostream *out;
  bool lvalue;

public:
  Generator(std::ostream &o, bool lvalue_) : out(&o), lvalue(lvalue_) {}

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

  void visit_add(const Add &n) final {
    if (lvalue)
      invalid(n);
    *this << "add(" << to_C_string(n.loc) << ", rule_name, " << to_C_string(n)
          << ", s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit_and(const And &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " && " << *n.rhs << ")";
  }

  void visit_band(const Band &n) final {
    if (lvalue)
      invalid(n);
    *this << "((value_t)(" << *n.lhs << " & " << *n.rhs << "))";
  }

  void visit_bnot(const Bnot &n) final {
    if (lvalue)
      invalid(n);
    *this << "bnot(" << *n.rhs << ")";
  }

  void visit_bor(const Bor &n) final {
    if (lvalue)
      invalid(n);
    *this << "((value_t)(" << *n.lhs << " | " << *n.rhs << "))";
  }

  void visit_div(const Div &n) final {
    if (lvalue)
      invalid(n);
    *this << "divide(" << to_C_string(n.loc) << ", rule_name, "
          << to_C_string(n) << ", s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit_element(const Element &n) final {
    if (lvalue && !n.is_lvalue())
      invalid(n);

    // First, determine the width of the array's elements

    const Ptr<TypeExpr> t1 = n.array->type();
    assert(t1 != nullptr && "array with invalid type");
    const Ptr<TypeExpr> t2 = t1->resolve();
    assert(t2 != nullptr && "array with invalid type");

    auto a = dynamic_cast<const Array &>(*t2);
    mpz_class element_width = a.element_type->width();

    // Second, determine the minimum and maximum values of the array's index
    // type

    const Ptr<TypeExpr> t3 = a.index_type->resolve();
    assert(t3 != nullptr && "array with invalid index type");

    mpz_class min, max;
    if (auto r = dynamic_cast<const Range *>(t3.get())) {
      min = r->min->constant_fold();
      max = r->max->constant_fold();
    } else if (auto e = dynamic_cast<const Enum *>(t3.get())) {
      min = 0;
      max = e->count() - 1;
    } else if (auto s = dynamic_cast<const Scalarset *>(t3.get())) {
      min = 0;
      max = s->bound->constant_fold() - 1;
    } else {
      assert(false && "array with invalid index type");
    }

    if (!lvalue && a.element_type->is_simple()) {
      const std::string lb = a.element_type->lower_bound();
      const std::string ub = a.element_type->upper_bound();
      *out << "handle_read(" << to_C_string(n.loc) << ", rule_name, "
           << to_C_string(n) << ", s, " << lb << ", " << ub << ", ";
    }

    *out << "handle_index(" << to_C_string(n.loc) << ", rule_name, "
         << to_C_string(n) << ", s, " << element_width << "ull, VALUE_C(" << min
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

  void visit_eq(const Eq &n) final {
    if (lvalue)
      invalid(n);

    if (!n.lhs->type()->is_simple()) {
      assert(!n.rhs->type()->is_simple() &&
             "comparison between simple and complex type");

      *this << "handle_eq(" << *n.lhs << ", " << *n.rhs << ")";

    } else {
      *this << "(" << *n.lhs << " == " << *n.rhs << ")";
    }
  }

  void visit_exists(const Exists &n) final {
    if (lvalue)
      invalid(n);

    *out << "({ bool result = false; ";
    generate_quantifier_header(*out, n.quantifier);
    *this << "if (" << *n.expr << ") { result = true; break; }";
    generate_quantifier_footer(*out, n.quantifier);
    *out << " result; })";
  }

  void visit_exprid(const ExprID &n) final {
    if (n.value == nullptr)
      throw Error("symbol \"" + n.id + "\" in expression is unresolved", n.loc);

    if (lvalue && !n.is_lvalue())
      invalid(n);

    /* This is a reference to a const. Note, this also covers enum
     * members.
     */
    if (auto c = dynamic_cast<const ConstDecl *>(n.value.get())) {
      assert(!lvalue && "const appearing as an lvalue");
      *out << "VALUE_C(" << c->value->constant_fold() << ")";
      return;
    }

    // This is either a state variable, a local variable or an alias.
    if (isa<AliasDecl>(n.value) || isa<VarDecl>(n.value)) {

      const Ptr<TypeExpr> t = n.type();
      assert((!n.is_lvalue() || t != nullptr) && "lvalue without a type");

      if (!lvalue && n.is_lvalue() && t->is_simple()) {
        const std::string lb = t->lower_bound();
        const std::string ub = t->upper_bound();
        *out << "handle_read(" << to_C_string(n.loc) << ", rule_name, "
             << to_C_string(n) << ", s, " << lb << ", " << ub << ", ";
      }

      *out << "ru_" << n.id;

      if (!lvalue && n.is_lvalue() && t->is_simple())
        *out << ")";
      return;
    }

    // FIXME: there's another case here where it's a reference to a quanitified
    // variable. I suspect we should just handle that the same way as a local.
  }

  void visit_field(const Field &n) final {
    if (lvalue && !n.is_lvalue())
      invalid(n);

    const Ptr<TypeExpr> root = n.record->type();
    assert(root != nullptr);
    const Ptr<TypeExpr> resolved = root->resolve();
    assert(resolved != nullptr);
    if (auto r = dynamic_cast<const Record *>(resolved.get())) {
      mpz_class offset = 0;
      for (const Ptr<VarDecl> &f : r->fields) {
        if (f->name == n.field) {
          if (!lvalue && f->type->is_simple()) {
            const std::string lb = f->type->lower_bound();
            const std::string ub = f->type->upper_bound();
            *out << "handle_read(" << to_C_string(n.loc) << ", rule_name, "
                 << to_C_string(n) << ", s, " << lb << ", " << ub << ", ";
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

  void visit_forall(const Forall &n) final {
    if (lvalue)
      invalid(n);

    *out << "({ bool result = true; ";
    generate_quantifier_header(*out, n.quantifier);
    *this << "if (!" << *n.expr << ") { result = false; break; }";
    generate_quantifier_footer(*out, n.quantifier);
    *out << " result; })";
  }

  void visit_functioncall(const FunctionCall &n) final {
    if (lvalue)
      invalid(n);

    if (n.function == nullptr)
      throw Error("unresolved function reference " + n.name, n.loc);

    const Ptr<TypeExpr> &return_type = n.function->return_type;

    // Open a statement-expression so we can declare temporaries.
    *out << "({ ";

    /* Secondly, a read-only value is never passed to a var parameter. This
     * should have been validated by FunctionCall::validate().
     */
    {
      auto it = n.function->parameters.begin();
      for (const Ptr<Expr> &a __attribute__((unused)) : n.arguments) {
        assert(((*it)->is_readonly() || !a->is_readonly()) &&
               "read-only value passed to var parameter");
        it++;
      }
    }

    /* Now for each parameter we need to consider five distinct methods, based
     * on the parameter's circumstance as described in the following table:
     *
     *   ┌──────┬────────────────┬─────────┬────────────╥────────┐
     *   │ var? │ simple/complex │ lvalue? │ read-only? ║ method │
     *   ├──────┼────────────────┼─────────┼────────────╫────────┤
     *   │  no  │     simple     │    no   │     -      ║    1   │
     *   │  no  │     simple     │   yes   │     no     ║    2   │
     *   │  no  │     simple     │   yes   │    yes     ║    2   │
     *   │  no  │    complex     │    no   │     -      ║    5   │
     *   │  no  │    complex     │   yes   │     no     ║    3   │
     *   │  no  │    complex     │   yes   │    yes     ║    3   │
     *   │ yes  │     simple     │    no   │     no     ║    1   │
     *   │ yes  │     simple     │   yes   │     no     ║    4   │
     *   │ yes  │    complex     │   yes   │     no     ║    4   │
     *   └──────┴────────────────┴─────────┴────────────╨────────┘
     *
     *   1. We can create a temporary handle and backing storage, then extract
     *      the value of the argument as an rvalue and write it to this
     *      temporary. The temporary can then be passed into the function,
     *      ensuring we don't modify the original argument.
     *
     *   2. We can do the same as (1), but extract the value of the argument
     *      with handle_read_raw. We need to do this because the argument might
     *      be undefined, in which case we want to extract its value without
     *      error. Another wrinkle we need to handle here is that the argument
     *      might be of a different range type than the function parameter type
     *      (differing lower and upper bounds).
     *
     *   3. We can create a temporary handle and backing store and then use
     *      handle_copy to transfer the value of the original argument. This is
     *      correct as we know the argument and the parameter it will be passed
     *      as have identical width.
     *
     *   4. We just pass the original handle, the lvalue of the argument.
     *
     *   5. We pass the original (rvalue) handle.
     */

    auto get_method =
      [](const Ptr<VarDecl> &parameter, const Ptr<Expr> &argument) {

        bool var = !parameter->is_readonly();
        bool simple = parameter->type->is_simple();
        bool is_lvalue = argument->is_lvalue();
        bool readonly = argument->is_readonly();

        if (!var &&  simple && !is_lvalue             ) return 1;
        if (!var &&  simple &&  is_lvalue && !readonly) return 2;
        if (!var &&  simple &&  is_lvalue &&  readonly) return 2;
        if (!var && !simple && !is_lvalue             ) return 5;
        if (!var && !simple &&  is_lvalue && !readonly) return 3;
        if (!var && !simple &&  is_lvalue &&  readonly) return 3;
        if ( var &&  simple && !is_lvalue             ) return 1;
        if ( var &&  simple &&  is_lvalue && !readonly) return 4;
        if ( var && !simple &&               !readonly) return 4;

        assert(!"unreachable");
        __builtin_unreachable();
      };

    // Create the temporaries for each argument.
    {
      size_t index = 0;
      auto it = n.function->parameters.begin();
      for (const Ptr<Expr> &a : n.arguments) {
        const Ptr<VarDecl> &p = *it;

        const std::string storage = "v" + std::to_string(n.unique_id) + "_" +
                                    std::to_string(index) + "_";
        const std::string handle =
            "v" + std::to_string(n.unique_id) + "_" + std::to_string(index);

        auto method = get_method(p, a);
        assert(method >= 1 && method <= 5);

        if (method == 1 || method == 2 || method == 3)
          *out << "uint8_t " << storage << "[BITS_TO_BYTES(" << p->width()
               << ")] = { 0 }; "
               << "struct handle " << handle << " = { .base = " << storage
               << ", .offset = 0, .width = " << p->width() << "ull }; ";

        if (method == 1) {
          const std::string lb = p->get_type()->lower_bound();
          const std::string ub = p->get_type()->upper_bound();

          *out << "handle_write(" << to_C_string(n.loc) << ", rule_name, "
               << "\"<temporary>\", s, " << lb << ", " << ub << ", " << handle
               << ", ";
          generate_rvalue(*out, *a);
          *out << "); ";

        } else if (method == 2) {
          const std::string lb = p->get_type()->lower_bound();
          const std::string ub = p->get_type()->upper_bound();

          const std::string lba = a->type()->lower_bound();

          *out << "{ "
               << "raw_value_t v = handle_read_raw(s, ";
          generate_lvalue(*out, *a);
          *out << "); "
               << "raw_value_t v2; "
               << "value_t v3; "
               << "static const value_t lb = " << lb << "; "
               << "static const value_t ub = " << ub << "; "
               << "if (v != 0 && (SUB(v, 1, &v2) || ADD(v2, " << lba
               << ", &v3) "
               << "|| v3 < lb || v3 > ub)) { "
               << "error(s, \"call to function %s passed an out-of-range value "
               << "%\" PRIRAWVAL \" to parameter " << (index + 1) << "\", \""
               << n.name << "\", raw_value_to_string(v + " << lba << " - 1)); "
               << "} "
               << "handle_write_raw(s, " << handle << ", v == 0 ? v : "
               << "((raw_value_t)(v3 - " << lb << ") + 1)); "
               << "} ";

        } else if (method == 3) {
          assert(a->type()->width() == p->width() &&
                 "complex function "
                 "parameter receiving an argument of a differing width");

          *out << "handle_copy(" << handle << ", ";
          generate_lvalue(*out, *a);
          *out << "); ";
        }

        it++;
        index++;
      }
    }

    *out << "ru_" << n.name << "(rule_name, state_drop_const(s)";

    // Pass the return type output parameter if required.
    if (return_type != nullptr && !return_type->is_simple())
      *out << ", (struct handle){ .base = ret" << n.unique_id
           << ", .offset = 0ul, .width = " << return_type->width() << "ull }";

    // Now emit the arguments to the function.
    {
      size_t index = 0;
      auto it = n.function->parameters.begin();
      for (const Ptr<Expr> &a : n.arguments) {

        *out << ", ";

        assert(it != n.function->parameters.end() &&
               "function call has more arguments than its target function");

        const Ptr<VarDecl> &p = *it;

        const std::string handle =
            "v" + std::to_string(n.unique_id) + "_" + std::to_string(index);

        switch (get_method(p, a)) {

        case 4:
          generate_lvalue(*out, *a);
          break;

        case 5:
          generate_rvalue(*out, *a);
          break;

        default:
          *out << handle;
          break;
        }

        index++;
        it++;
      }
    }

    *out << ");";

    // Close the statement-expression.
    *out << " })";
  }

  void visit_geq(const Geq &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " >= " << *n.rhs << ")";
  }

  void visit_gt(const Gt &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " > " << *n.rhs << ")";
  }

  void visit_implication(const Implication &n) final {
    if (lvalue)
      invalid(n);
    *this << "(!" << *n.lhs << " || " << *n.rhs << ")";
  }

  void visit_isundefined(const IsUndefined &n) final {
    *this << "handle_isundefined(s, ";
    generate_lvalue(*out, *n.rhs);
    *this << ")";
  }

  void visit_leq(const Leq &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " <= " << *n.rhs << ")";
  }

  void visit_lsh(const Lsh &n) final {
    if (lvalue)
      invalid(n);
    *this << "lsh(" << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit_lt(const Lt &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " < " << *n.rhs << ")";
  }

  void visit_mod(const Mod &n) final {
    if (lvalue)
      invalid(n);
    *this << "mod(" << to_C_string(n.loc) << ", rule_name, " << to_C_string(n)
          << ", s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit_mul(const Mul &n) final {
    if (lvalue)
      invalid(n);
    *this << "mul(" << to_C_string(n.loc) << ", rule_name, " << to_C_string(n)
          << ", s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit_negative(const Negative &n) final {
    if (lvalue)
      invalid(n);

    // if this is negation of something that may be `VALUE_MIN`, assume it
    // cannot be printed as-is without UB
    auto constant = dynamic_cast<const Number *>(n.rhs.get());
    if (constant != nullptr) {
      if (-constant->value == INT8_MIN) {
        *out << "((value_t)INT8_MIN)";
        return;
      }
      if (-constant->value == INT16_MIN) {
        *out << "((value_t)INT16_MIN)";
        return;
      }
      if (-constant->value == INT32_MIN) {
        *out << "((value_t)INT32_MIN)";
        return;
      }
      if (-constant->value == mpz_class{std::to_string(INT64_MIN)}) {
        *out << "((value_t)INT64_MIN)";
        return;
      }
    }

    *this << "negate(" << to_C_string(n.loc) << ", rule_name, "
          << to_C_string(n) << ", s, " << *n.rhs << ")";
  }

  void visit_neq(const Neq &n) final {
    if (lvalue)
      invalid(n);

    if (!n.lhs->type()->is_simple()) {
      assert(!n.rhs->type()->is_simple() &&
             "comparison between simple and complex type");

      *this << "(!handle_eq(" << *n.lhs << ", " << *n.rhs << "))";

    } else {
      *this << "(" << *n.lhs << " != " << *n.rhs << ")";
    }
  }

  void visit_not(const Not &n) final {
    if (lvalue)
      invalid(n);
    *this << "(!" << *n.rhs << ")";
  }

  void visit_number(const Number &n) final {
    *out << "VALUE_C(" << n.value << ")";
  }

  void visit_or(const Or &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.lhs << " || " << *n.rhs << ")";
  }

  void visit_rsh(const Rsh &n) final {
    if (lvalue)
      invalid(n);
    *this << "rsh(" << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit_sub(const Sub &n) final {
    if (lvalue)
      invalid(n);
    *this << "sub(" << to_C_string(n.loc) << ", rule_name, " << to_C_string(n)
          << ", s, " << *n.lhs << ", " << *n.rhs << ")";
  }

  void visit_ternary(const Ternary &n) final {
    if (lvalue)
      invalid(n);
    *this << "(" << *n.cond << " ? " << *n.lhs << " : " << *n.rhs << ")";
  }

  void visit_xor(const Xor &n) final {
    if (lvalue)
      invalid(n);
    *this << "((value_t)(" << *n.lhs << " ^ " << *n.rhs << "))";
  }

  virtual ~Generator() = default;

private:
  void invalid(const Expr &n) const {
    throw Error("invalid expression used as lvalue", n.loc);
  }
};

} // namespace

void generate_lvalue(std::ostream &out, const Expr &e) {
  Generator g(out, true);
  g.dispatch(e);
}

void generate_rvalue(std::ostream &out, const Expr &e) {
  Generator g(out, false);
  g.dispatch(e);
}
