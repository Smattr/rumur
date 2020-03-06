#include <cassert>
#include <cstddef>
#include "CLikeGenerator.h"
#include <iostream>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

void CLikeGenerator::visit_add(const Add &n) {
  *this << "(" << *n.lhs << " + " << *n.rhs << ")";
}

void CLikeGenerator::visit_and(const And &n) {
  *this << "(" << *n.lhs << " && " << *n.rhs << ")";
}

void CLikeGenerator::visit_div(const Div &n) {
  *this << "(" << *n.lhs << " / " << *n.rhs << ")";
}

void CLikeGenerator::visit_element(const Element &n) {
  *this << "(" << *n.array << ".data[" << *n.index << "])";
}

void CLikeGenerator::visit_eq(const Eq &n) {

  if (!n.lhs->type()->is_simple()) {
    // This is a comparison of an array or struct. We cannot use the built-in
    // == operator, so we use memcmp. This only works if all members are
    // packed, hence why `__attribute__((pack))` is emitted in other places.
    assert(pack && "comparison of complex types is present but structures "
      "are not packed");
    *this << "(memcmp(&" << *n.lhs << ", &" << *n.rhs << ", sizeof" << *n.lhs
      << ") == 0)";

    return;
  }

  *this << "(" << *n.lhs << " == " << *n.rhs << ")";
}

void CLikeGenerator::visit_exists(const Exists &n) {
  *this << "({ bool res_ = false; " << n.quantifier << " { res_ |= "
    << *n.expr << "; } res_; })";
}

void CLikeGenerator::visit_exprid(const ExprID &n) {
  *this << "(" << n.id << ")";
}

void CLikeGenerator::visit_field(const Field &n) {
  *this << "(" << *n.record << "." << n.field << ")";
}

void CLikeGenerator::visit_forall(const Forall &n) {

  // open a GNU statement expression
  *this << "({ ";

  // see corresponding logic in visit_for() for an explanation
  if (auto e = dynamic_cast<const Enum*>(n.quantifier.type.get())) {
    *this << *e << "; ";
  }

  *this << "bool res_ = true; " << n.quantifier << " { res_ &= "
    << *n.expr << "; } res_; })";
}

void CLikeGenerator::visit_functioncall(const FunctionCall &n) {
  *this << n.name << "(";
  assert(n.function != nullptr && "unresolved function call in AST");
  auto it = n.function->parameters.begin();
  bool first = true;
  for (const Ptr<Expr> &a : n.arguments) {
    if (!first) {
      *this << ", ";
    }
    if (!(*it)->readonly) {
      *this << "&";
    }
    *this << *a;
    first = false;
    it++;
  }
  *this << ")";
}

void CLikeGenerator::visit_geq(const Geq &n) {
  *this << "(" << *n.lhs << " >= " << *n.rhs << ")";
}

void CLikeGenerator::visit_gt(const Gt &n) {
  *this << "(" << *n.lhs << " > " << *n.rhs << ")";
}

void CLikeGenerator::visit_implication(const Implication &n) {
  *this << "(!" << *n.lhs << " || " << *n.rhs << ")";
}

void CLikeGenerator::visit_isundefined(const IsUndefined&) {
  // check() prevents a model with isundefined expressions from making it
  // through to here
  assert(!"unreachable");
  __builtin_unreachable();
}

void CLikeGenerator::visit_leq(const Leq &n) {
  *this << "(" << *n.lhs << " <= " << *n.rhs << ")";
}

void CLikeGenerator::visit_lt(const Lt &n) {
  *this << "(" << *n.lhs << " < " << *n.rhs << ")";
}

void CLikeGenerator::visit_mod(const Mod &n) {
  *this << "(" << *n.lhs << " % " << *n.rhs << ")";
}

void CLikeGenerator::visit_mul(const Mul &n) {
  *this << "(" << *n.lhs << " * " << *n.rhs << ")";
}

void CLikeGenerator::visit_negative(const Negative &n) {
  *this << "(-" << *n.rhs << ")";
}

void CLikeGenerator::visit_neq(const Neq &n) {

  if (!n.lhs->type()->is_simple()) {
    // see explanation in visit_eq()
    assert(pack && "comparison of complex types is present but structures "
      "are not packed");
    *this << "(memcmp(&" << *n.lhs << ", &" << *n.rhs << ", sizeof" << *n.lhs
      << ") != 0)";

    return;
  }

  *this << "(" << *n.lhs << " != " << *n.rhs << ")";
}

void CLikeGenerator::visit_not(const Not &n) {
  *this << "(!" << *n.rhs << ")";
}

void CLikeGenerator::visit_number(const Number &n) {
  *this << "(INT64_C(" << n.value.get_str() << "))";
}

void CLikeGenerator::visit_or(const Or &n) {
  *this << "(" << *n.lhs << " || " << *n.rhs << ")";
}

void CLikeGenerator::visit_procedurecall(const ProcedureCall &n) {
  *this << indentation() << n.call << ";\n";
}

void CLikeGenerator::visit_property(const Property&) {
  // this is unreachable because generate_c is only ever called with a Model
  // and nothing that contains a Property descends into it
  assert(!"unreachable");
  __builtin_unreachable();
}

void CLikeGenerator::visit_sub(const Sub &n) {
  *this << "(" << *n.lhs << " - " << *n.rhs << ")";
}

void CLikeGenerator::visit_ternary(const Ternary &n) {
  *this << "(" << *n.cond << " ? " << *n.lhs << " : " << *n.rhs << ")";
}

void CLikeGenerator::visit_undefine(const Undefine &n) {
  *this << indentation() << "memset(&" << *n.rhs << ", 0, sizeof(" << *n.rhs
    << "));\n";
}

CLikeGenerator &CLikeGenerator::operator<<(const std::string &s) {
  out << s;
  return *this;
}

CLikeGenerator &CLikeGenerator::operator<<(const Node &n) {
  dispatch(n);
  return *this;
}

CLikeGenerator::~CLikeGenerator() { }
