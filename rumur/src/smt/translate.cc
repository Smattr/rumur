#include <cstddef>
#include <rumur/rumur.h>
#include "except.h"
#include "logic.h"
#include "../options.h"
#include "translate.h"
#include <sstream>
#include <string>

using namespace rumur;

namespace smt {

namespace { class Translator : public ConstExprTraversal {

 private:
  std::ostringstream buffer;
  const Logic *logic;

 public:
  Translator(const Logic &logic_): logic(&logic_) { }

  std::string str() const {
    return buffer.str();
  }

  Translator &operator<<(const std::string &s) {
    buffer << s;
    return *this;
  }

  Translator &operator<<(const Expr &e) {
    dispatch(e);
    return *this;
  }

  void visit_add(const Add &n) {
    *this << "(" << logic->add() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_and(const And &n) {
    *this << "(and " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_element(const Element &n) {
    throw Unsupported(n);
  }

  void visit_exprid(const ExprID &n) {
    *this << mangle(n.id);
  }

  void visit_eq(const Eq &n) {
    *this << "(= " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_exists(const Exists &n) {
    throw Unsupported(n);
  }

  void visit_div(const Div &n) {
    *this << "(" << logic->div() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_field(const Field &n) {
    throw Unsupported(n);
  }

  void visit_forall(const Forall &n) {
    throw Unsupported(n);
  }

  void visit_functioncall(const FunctionCall &n) {
    throw Unsupported(n);
  }

  void visit_geq(const Geq &n) {
    *this << "(" << logic->geq() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_gt(const Gt &n) {
    *this << "(" << logic->gt() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_implication(const Implication &n) {
    *this << "(=> " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_isundefined(const IsUndefined &n) {
    throw Unsupported(n);
  }

  void visit_leq(const Leq &n) {
    *this << "(" << logic->leq() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_lt(const Lt &n) {
    *this << "(" << logic->lt() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_mod(const Mod &n) {
    *this << "(" << logic->mod() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_mul(const Mul &n) {
    *this << "(" << logic->mul() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_negative(const Negative &n) {
    *this << "(" << logic->neg() << " " << *n.rhs << ")";
  }

  void visit_neq(const Neq &n) {
    *this << "(not (= " << *n.lhs << " " << *n.rhs << "))";
  }

  void visit_number(const Number &n) {
    *this << logic->numeric_literal(n.value);
  }

  void visit_not(const Not &n) {
    *this << "(not " << *n.rhs << ")";
  }

  void visit_or(const Or &n) {
    *this << "(or " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_sub(const Sub &n) {
    *this << "(" << logic->sub() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_ternary(const Ternary &n) {
    *this << "(ite " << *n.cond << " " << *n.lhs << " " << *n.rhs << ")";
  }
}; }

std::string translate(const Expr &expr) {
  const Logic &logic = get_logic(options.smt.logic);

  Translator t(logic);
  t.dispatch(expr);
  return t.str();
}

std::string mangle(const std::string &s) {
  return "ru_" + s;
}

}
