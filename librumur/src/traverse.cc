#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <typeinfo>

namespace rumur {

void BaseTraversal::dispatch(Node &n) {

  if (auto i = dynamic_cast<Add*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<AliasDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<AliasRule*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<AliasStmt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<And*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Array*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Assignment*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<ConstDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Clear*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Div*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Element*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Function*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<FunctionCall*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Enum*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Eq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<ErrorStmt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Exists*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<ExprID*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Field*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<For*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Forall*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Geq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Gt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<If*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<IfClause*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Implication*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Leq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Lt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Model*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Mod*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Mul*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Negative*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Neq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Not*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Number*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Or*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<ProcedureCall*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Property*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<PropertyRule*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<PropertyStmt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Put*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Quantifier*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Range*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Record*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Return*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Ruleset*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Scalarset*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<SimpleRule*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<StartState*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Sub*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Switch*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<SwitchCase*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Ternary*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<TypeDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<TypeExprID*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Undefine*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<VarDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<While*>(&n)) {
    visit(*i);
    return;
  }

#ifndef NDEBUG
  std::cerr << "missed case in BaseTraversal::dispatch: " << typeid(n).name() << "\n";
#endif
  assert(!"missed case in BaseTraversal::dispatch");
}

void ConstBaseTraversal::dispatch(const Node &n) {

  if (auto i = dynamic_cast<const Add*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const AliasDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const AliasRule*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const AliasStmt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const And*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Array*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Assignment*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Clear*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const ConstDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Div*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Element*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Enum*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Eq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const ErrorStmt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Exists*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const ExprID*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Field*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const For*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Forall*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Function*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const FunctionCall*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Geq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Gt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const If*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const IfClause*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Implication*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Leq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Lt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Model*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Mod*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Mul*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Negative*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Neq*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Not*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Number*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Or*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const ProcedureCall*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Property*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const PropertyRule*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const PropertyStmt*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Put*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Quantifier*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Range*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Record*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Return*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Ruleset*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Scalarset*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const SimpleRule*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const StartState*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Sub*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Switch*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const SwitchCase*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Ternary*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const TypeDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const TypeExprID*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const Undefine*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const VarDecl*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<const While*>(&n)) {
    visit(*i);
    return;
  }

#ifndef NDEBUG
  std::cerr << "missed case in ConstBaseTraversal::dispatch: " << typeid(n).name() << "\n";
#endif
  assert(!"missed case in ConstBaseTraversal::dispatch");
}

void ConstTraversal::visit(const Add &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstTraversal::visit(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit(const AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const And &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void ConstTraversal::visit(const Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit_bexpr(const BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const Clear &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstTraversal::visit(const Div &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void ConstTraversal::visit(const Enum&) { }

void ConstTraversal::visit(const Eq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const ErrorStmt&) { }

void ConstTraversal::visit(const Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTraversal::visit(const ExprID&) { }

void ConstTraversal::visit(const Field &n) {
  dispatch(*n.record);
}

void ConstTraversal::visit(const For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTraversal::visit(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

void ConstTraversal::visit(const Geq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Gt &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const If &n) {
  for (const IfClause &c : n.clauses)
    dispatch(c);
}

void ConstTraversal::visit(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const Implication &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Leq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Lt &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Mod &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit(const Mul &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Negative &n) {
  visit_uexpr(static_cast<const UnaryExpr&>(n));
}

void ConstTraversal::visit(const Neq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Not &n) {
  visit_uexpr(static_cast<const UnaryExpr&>(n));
}

void ConstTraversal::visit(const Number&) { }

void ConstTraversal::visit(const Or &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const ProcedureCall &n) {
  dispatch(n.call);
}

void ConstTraversal::visit(const Property &n) {
  dispatch(*n.expr);
}

void ConstTraversal::visit(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstTraversal::visit(const PropertyStmt &n) {
  dispatch(n.property);
}

void ConstTraversal::visit(const Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTraversal::visit(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstTraversal::visit(const Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void ConstTraversal::visit(const Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

void ConstTraversal::visit(const Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTraversal::visit(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit(const Scalarset &n) {
  dispatch(*n.bound);
}

void ConstTraversal::visit(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const Sub &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Switch &n) {
  dispatch(*n.expr);
  for (const SwitchCase &c : n.cases)
    dispatch(c);
}

void ConstTraversal::visit(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstTraversal::visit(const TypeExprID&) { }

void ConstTraversal::visit_uexpr(const UnaryExpr &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const Undefine &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstTraversal::visit(const While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

ConstTraversal::~ConstTraversal() { }

void ConstExprTraversal::visit(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstExprTraversal::visit(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstExprTraversal::visit(const AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit(const Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void ConstExprTraversal::visit(const Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstExprTraversal::visit(const Clear &n) {
  dispatch(*n.rhs);
}

void ConstExprTraversal::visit(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstExprTraversal::visit(const Enum&) { }

void ConstExprTraversal::visit(const ErrorStmt&) { }

void ConstExprTraversal::visit(const For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit(const If &n) {
  for (const IfClause &c : n.clauses)
    dispatch(c);
}

void ConstExprTraversal::visit(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstExprTraversal::visit(const ProcedureCall &n) {
  dispatch(n.call);
}

void ConstExprTraversal::visit(const Property &n) {
  dispatch(*n.expr);
}

void ConstExprTraversal::visit(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstExprTraversal::visit(const PropertyStmt &n) {
  dispatch(n.property);
}

void ConstExprTraversal::visit(const Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstExprTraversal::visit(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstExprTraversal::visit(const Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void ConstExprTraversal::visit(const Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

void ConstExprTraversal::visit(const Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstExprTraversal::visit(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstExprTraversal::visit(const Scalarset &n) {
  dispatch(*n.bound);
}

void ConstExprTraversal::visit(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit(const Switch &n) {
  dispatch(*n.expr);
  for (const SwitchCase &c : n.cases)
    dispatch(c);
}

void ConstExprTraversal::visit(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstExprTraversal::visit(const TypeExprID&) { }

void ConstExprTraversal::visit(const Undefine &n) {
  dispatch(*n.rhs);
}

void ConstExprTraversal::visit(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstExprTraversal::visit(const While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstStmtTraversal::visit(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstStmtTraversal::visit(const Add &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const And &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void ConstStmtTraversal::visit_bexpr(const BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstStmtTraversal::visit(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstStmtTraversal::visit(const Div &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void ConstStmtTraversal::visit(const Enum&) { }

void ConstStmtTraversal::visit(const Eq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstStmtTraversal::visit(const ExprID&) { }

void ConstStmtTraversal::visit(const Field &n) {
  dispatch(*n.record);
}

void ConstStmtTraversal::visit(const Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstStmtTraversal::visit(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit(const FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

void ConstStmtTraversal::visit(const Geq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Gt &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit(const Implication &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Leq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Lt &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Mod &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstStmtTraversal::visit(const Mul &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Negative &n) {
  visit_uexpr(static_cast<const UnaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Neq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Not &n) {
  visit_uexpr(static_cast<const UnaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Number&) { }

void ConstStmtTraversal::visit(const Or &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Property &n) {
  dispatch(*n.expr);
}

void ConstStmtTraversal::visit(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstStmtTraversal::visit(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstStmtTraversal::visit(const Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void ConstStmtTraversal::visit(const Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

void ConstStmtTraversal::visit(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstStmtTraversal::visit(const Scalarset &n) {
  dispatch(*n.bound);
}

void ConstStmtTraversal::visit(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit(const Sub &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstStmtTraversal::visit(const Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstStmtTraversal::visit(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstStmtTraversal::visit(const TypeExprID&) { }

void ConstStmtTraversal::visit_uexpr(const UnaryExpr &n) {
  dispatch(*n.rhs);
}

void ConstStmtTraversal::visit(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstTypeTraversal::visit(const Add &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstTypeTraversal::visit(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTypeTraversal::visit(const AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit(const And &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit_bexpr(const BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit(const Clear &n) {
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstTypeTraversal::visit(const Div &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void ConstTypeTraversal::visit(const Eq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const ErrorStmt&) { }

void ConstTypeTraversal::visit(const Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTypeTraversal::visit(const ExprID&) { }

void ConstTypeTraversal::visit(const Field &n) {
  dispatch(*n.record);
}

void ConstTypeTraversal::visit(const For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit(const Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTypeTraversal::visit(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit(const FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

void ConstTypeTraversal::visit(const Geq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Gt &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const If &n) {
  for (const IfClause &c : n.clauses)
    dispatch(c);
}

void ConstTypeTraversal::visit(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit(const Implication &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Leq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Lt &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Mod &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTypeTraversal::visit(const Mul &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Negative &n) {
  visit_uexpr(static_cast<const UnaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Neq &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Not &n) {
  visit_uexpr(static_cast<const UnaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Number&) { }

void ConstTypeTraversal::visit(const Or &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const ProcedureCall &n) {
  dispatch(n.call);
}

void ConstTypeTraversal::visit(const Property &n) {
  dispatch(*n.expr);
}

void ConstTypeTraversal::visit(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstTypeTraversal::visit(const PropertyStmt &n) {
  dispatch(n.property);
}

void ConstTypeTraversal::visit(const Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTypeTraversal::visit(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstTypeTraversal::visit(const Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTypeTraversal::visit(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTypeTraversal::visit(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit(const Sub &n) {
  visit_bexpr(static_cast<const BinaryExpr&>(n));
}

void ConstTypeTraversal::visit(const Switch &n) {
  dispatch(*n.expr);
  for (const SwitchCase &c : n.cases)
    dispatch(c);
}

void ConstTypeTraversal::visit(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit(const Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstTypeTraversal::visit_uexpr(const UnaryExpr &n) {
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit(const Undefine &n) {
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstTypeTraversal::visit(const While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

}
