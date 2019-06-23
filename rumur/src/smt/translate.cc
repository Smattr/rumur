#include <cstddef>
#include <rumur/rumur.h>
#include "except.h"
#include "translate.h"
#include <sstream>
#include <string>

using namespace rumur;

namespace smt {

namespace { class Translator : public ConstExprTraversal {

 private:
  std::ostringstream buffer;

 public:
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
    *this << "(+ " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_and(const And &n) {
    *this << "(and " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_element(const Element&) {
    throw Unsupported();
  }

  void visit_exprid(const ExprID &n) {
    *this << n.id;
  }

  void visit_eq(const Eq &n) {
    *this << "(= " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_exists(const Exists&) {
    throw Unsupported();
  }

  void visit_div(const Div &n) {
    *this << "(div " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_field(const Field&) {
    throw Unsupported();
  }

  void visit_forall(const Forall&) {
    throw Unsupported();
  }

  void visit_functioncall(const FunctionCall&) {
    throw Unsupported();
  }

  void visit_geq(const Geq &n) {
    *this << "(>= " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_gt(const Gt &n) {
    *this << "(> " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_implication(const Implication &n) {
    *this << "(=> " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_isundefined(const IsUndefined&) {
    throw Unsupported();
  }

  void visit_leq(const Leq &n) {
    *this << "(<= " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_lt(const Lt &n) {
    *this << "(< " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_mod(const Mod &n) {
    *this << "(mod " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_mul(const Mul &n) {
    *this << "(* " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_negative(const Negative &n) {
    *this << "(- " << *n.rhs << ")";
  }

  void visit_neq(const Neq &n) {
    *this << "(not (= " << *n.lhs << " " << *n.rhs << "))";
  }

  void visit_number(const Number &n) {
    *this << n.value.get_str();
  }

  void visit_not(const Not &n) {
    *this << "(not " << *n.rhs << ")";
  }

  void visit_or(const Or &n) {
    *this << "(or " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_sub(const Sub &n) {
    *this << "(sub " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_ternary(const Ternary &n) {
    *this << "(ite " << *n.cond << " " << *n.lhs << " " << *n.rhs << ")";
  }
}; }

std::string translate(const Expr &expr) {
  Translator t;
  t.dispatch(expr);
  return t.str();
}

}
