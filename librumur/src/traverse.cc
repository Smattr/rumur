#include <cassert>
#include <iostream>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <typeinfo>

namespace rumur {

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
  dispatch(*n.value);
}

void Traversal::visit(Field &n) {
  dispatch(*n.record);
}

void Traversal::visit(For &n) {
  dispatch(*n.quantifier);
  for (Stmt *s : n.body)
    dispatch(*s);
}

void Traversal::visit(Forall &n) {
  dispatch(*n.quantifier);
  dispatch(*n.expr);
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
  for (Stmt *s : n.body)
    dispatch(*s);
}

void Traversal::visit(Implication &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Invariant &n) {
  for (Quantifier *q : n.quantifiers)
    dispatch(*q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
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
  for (Decl *d : n.decls)
    dispatch(*d);
  for (Rule *r : n.rules)
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

void Traversal::dispatch(Node &n) {

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

  if (auto i = dynamic_cast<Div*>(&n)) {
    visit(*i);
    return;
  }

  if (auto i = dynamic_cast<Element*>(&n)) {
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

  if (auto i = dynamic_cast<Invariant*>(&n)) {
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

  if (auto i = dynamic_cast<Property*>(&n)) {
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
  std::cerr << "missed case in Traversal::visit: " << typeid(n).name() << "\n";
#endif
  assert(!"missed case in Traversal::visit");
}

void Traversal::visit(Not &n) {
  visit(static_cast<UnaryExpr&>(n));
}

void Traversal::visit(Number&) { }

void Traversal::visit(Or &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Property &n) {
  dispatch(*n.expr);
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
  for (VarDecl *f : n.fields)
    dispatch(*f);
}

void Traversal::visit(Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Traversal::visit(Ruleset &n) {
  for (Quantifier *q : n.quantifiers)
    dispatch(*q);
  for (Rule *r : n.rules)
    dispatch(*r);
}

void Traversal::visit(Scalarset &n) {
  dispatch(*n.bound);
}

void Traversal::visit(SimpleRule &n) {
  for (Quantifier *q : n.quantifiers)
    dispatch(*q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (Decl *d : n.decls)
    dispatch(*d);
  for (Stmt *s : n.body)
    dispatch(*s);
}

void Traversal::visit(StartState &n) {
  for (Quantifier *q : n.quantifiers)
    dispatch(*q);
  for (Decl *d : n.decls)
    dispatch(*d);
  for (Stmt *s : n.body)
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

}
