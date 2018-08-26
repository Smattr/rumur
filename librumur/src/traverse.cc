#include <cassert>
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

  if (auto i = dynamic_cast<Parameter*>(&n)) {
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

#ifndef NDEBUG
  std::cerr << "missed case in BaseTraversal::visit: " << typeid(n).name() << "\n";
#endif
  assert(!"missed case in BaseTraversal::visit");
}

void Traversal::visit(Add &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(And &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void Traversal::visit(Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Traversal::visit(BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Traversal::visit(Clear &n) {
  dispatch(*n.rhs);
}

void Traversal::visit(ConstDecl &n) {
  dispatch(*n.value);
}

void Traversal::visit(Div &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void Traversal::visit(Enum&) { }

void Traversal::visit(Eq &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(ErrorStmt&) { }

void Traversal::visit(Exists &n) {
  dispatch(*n.quantifier);
  dispatch(*n.expr);
}

void Traversal::visit(ExprID &n) {
  if (n.value != nullptr)
    dispatch(*n.value);
}

void Traversal::visit(Field &n) {
  dispatch(*n.record);
}

void Traversal::visit(For &n) {
  dispatch(*n.quantifier);
  for (std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void Traversal::visit(Forall &n) {
  dispatch(*n.quantifier);
  dispatch(*n.expr);
}

void Traversal::visit(Function &n) {
  for (std::shared_ptr<Parameter> &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void Traversal::visit(FunctionCall &n) {
  dispatch(*n.function);
  for (std::shared_ptr<Expr> &a : n.arguments)
    dispatch(*a);
}

void Traversal::visit(Geq &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Gt &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(If &n) {
  for (IfClause &c : n.clauses)
    dispatch(c);
}

void Traversal::visit(IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void Traversal::visit(Implication &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Leq &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Lt &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Mod &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Model &n) {
  for (std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (std::shared_ptr<Function> &f : n.functions)
    dispatch(*f);
  for (std::shared_ptr<Rule> &r : n.rules)
    dispatch(*r);
}

void Traversal::visit(Mul &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Negative &n) {
  visit(static_cast<UnaryExpr&>(n));
}

void Traversal::visit(Neq &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Not &n) {
  visit(static_cast<UnaryExpr&>(n));
}

void Traversal::visit(Number&) { }

void Traversal::visit(Or &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Parameter &n) {
  dispatch(*n.decl);
}

void Traversal::visit(ProcedureCall &n) {
  dispatch(*n.function);
  for (std::shared_ptr<Expr> &a : n.arguments)
    dispatch(*a);
}

void Traversal::visit(Property &n) {
  dispatch(*n.expr);
}

void Traversal::visit(PropertyRule &n) {
  for (std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  dispatch(n.property);
}

void Traversal::visit(PropertyStmt &n) {
  dispatch(n.property);
}

void Traversal::visit(Quantifier &n) {
  dispatch(*n.var);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void Traversal::visit(Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void Traversal::visit(Record &n) {
  for (std::shared_ptr<VarDecl> &f : n.fields)
    dispatch(*f);
}

void Traversal::visit(Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Traversal::visit(Ruleset &n) {
  for (std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  for (std::shared_ptr<Rule> &r : n.rules)
    dispatch(*r);
}

void Traversal::visit(Scalarset &n) {
  dispatch(*n.bound);
}

void Traversal::visit(SimpleRule &n) {
  for (std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void Traversal::visit(StartState &n) {
  for (std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  for (std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void Traversal::visit(Sub &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Traversal::visit(TypeDecl &n) {
  dispatch(*n.value);
}

void Traversal::visit(TypeExprID &n) {
  if (n.referent != nullptr)
    dispatch(*n.referent);
}

void Traversal::visit(UnaryExpr &n) {
  dispatch(*n.rhs);
}

void Traversal::visit(Undefine &n) {
  dispatch(*n.rhs);
}

void Traversal::visit(VarDecl &n) {
  dispatch(*n.type);
}

Traversal::~Traversal() { }

void ConstBaseTraversal::dispatch(const Node &n) {

  if (auto i = dynamic_cast<const Add*>(&n)) {
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

  if (auto i = dynamic_cast<const Parameter*>(&n)) {
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

#ifndef NDEBUG
  std::cerr << "missed case in ConstBaseTraversal::visit: " << typeid(n).name() << "\n";
#endif
  assert(!"missed case in ConstBaseTraversal::visit");
}

void ConstTraversal::visit(const Add &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const And &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void ConstTraversal::visit(const Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const BinaryExpr &n) {
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
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void ConstTraversal::visit(const Enum&) { }

void ConstTraversal::visit(const Eq &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const ErrorStmt&) { }

void ConstTraversal::visit(const Exists &n) {
  dispatch(*n.quantifier);
  dispatch(*n.expr);
}

void ConstTraversal::visit(const ExprID &n) {
  if (n.value != nullptr)
    dispatch(*n.value);
}

void ConstTraversal::visit(const Field &n) {
  dispatch(*n.record);
}

void ConstTraversal::visit(const For &n) {
  dispatch(*n.quantifier);
  for (const std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const Forall &n) {
  dispatch(*n.quantifier);
  dispatch(*n.expr);
}

void ConstTraversal::visit(const Function &n) {
  for (const std::shared_ptr<Parameter> &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (const std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (const std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const FunctionCall &n) {
  dispatch(*n.function);
  for (const std::shared_ptr<Expr> &a : n.arguments)
    dispatch(*a);
}

void ConstTraversal::visit(const Geq &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Gt &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const If &n) {
  for (const IfClause &c : n.clauses)
    dispatch(c);
}

void ConstTraversal::visit(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (const std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const Implication &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Leq &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Lt &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Mod &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Model &n) {
  for (const std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (const std::shared_ptr<Function> &f : n.functions)
    dispatch(*f);
  for (const std::shared_ptr<Rule> &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit(const Mul &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Negative &n) {
  visit(static_cast<const UnaryExpr&>(n));
}

void ConstTraversal::visit(const Neq &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Not &n) {
  visit(static_cast<const UnaryExpr&>(n));
}

void ConstTraversal::visit(const Number&) { }

void ConstTraversal::visit(const Or &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Parameter &n) {
  dispatch(*n.decl);
}

void ConstTraversal::visit(const ProcedureCall &n) {
  dispatch(*n.function);
  for (const std::shared_ptr<Expr> &a : n.arguments)
    dispatch(*a);
}

void ConstTraversal::visit(const Property &n) {
  dispatch(*n.expr);
}

void ConstTraversal::visit(const PropertyRule &n) {
  for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  dispatch(n.property);
}

void ConstTraversal::visit(const PropertyStmt &n) {
  dispatch(n.property);
}

void ConstTraversal::visit(const Quantifier &n) {
  dispatch(*n.var);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstTraversal::visit(const Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void ConstTraversal::visit(const Record &n) {
  for (const std::shared_ptr<VarDecl> &f : n.fields)
    dispatch(*f);
}

void ConstTraversal::visit(const Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTraversal::visit(const Ruleset &n) {
  for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  for (const std::shared_ptr<Rule> &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit(const Scalarset &n) {
  dispatch(*n.bound);
}

void ConstTraversal::visit(const SimpleRule &n) {
  for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (const std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (const std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const StartState &n) {
  for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
    dispatch(*q);
  for (const std::shared_ptr<Decl> &d : n.decls)
    dispatch(*d);
  for (const std::shared_ptr<Stmt> &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit(const Sub &n) {
  visit(static_cast<const BinaryExpr&>(n));
}

void ConstTraversal::visit(const Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstTraversal::visit(const TypeExprID &n) {
  if (n.referent != nullptr)
    dispatch(*n.referent);
}

void ConstTraversal::visit(const UnaryExpr &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const Undefine &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit(const VarDecl &n) {
  dispatch(*n.type);
}

ConstTraversal::~ConstTraversal() { }

}
