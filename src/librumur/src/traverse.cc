#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>

namespace rumur {

void Traversal::visit(Add &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(And &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Array &n) {
  visit(*n.index_type);
  visit(*n.element_type);
}

void Traversal::visit(Assert &n) {
  visit(*n.expr);
}

void Traversal::visit(Assignment &n) {
  visit(*n.lhs);
  visit(*n.rhs);
}

void Traversal::visit(BinaryExpr &n) {
  visit(*n.lhs);
  visit(*n.rhs);
}

void Traversal::visit(ConstDecl &n) {
  visit(*n.value);
}

void Traversal::visit(Div &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Element &n) {
  visit(*n.array);
  visit(*n.index);
}

void Traversal::visit(Enum&) { }

void Traversal::visit(Eq &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(ErrorStmt&) { }

void Traversal::visit(Exists &n) {
  visit(*n.quantifier);
  visit(*n.expr);
}

void Traversal::visit(ExprID &n) {
  visit(*n.value);
}

void Traversal::visit(Field &n) {
  visit(*n.record);
}

void Traversal::visit(For &n) {
  visit(*n.quantifier);
  for (Stmt *s : n.body)
    visit(*s);
}

void Traversal::visit(Forall &n) {
  visit(*n.quantifier);
  visit(*n.expr);
}

void Traversal::visit(Geq &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Gt &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(If &n) {
  for (IfClause &c : n.clauses)
    visit(c);
}

void Traversal::visit(IfClause &n) {
  if (n.condition != nullptr)
    visit(*n.condition);
  for (Stmt *s : n.body)
    visit(*s);
}

void Traversal::visit(Implication &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Invariant &n) {
  for (Quantifier *q : n.quantifiers)
    visit(*q);
  if (n.guard != nullptr)
    visit(*n.guard);
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
    visit(*d);
  for (Rule *r : n.rules)
    visit(*r);
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

void Traversal::visit(Node &n) {

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

  if (auto i = dynamic_cast<Assert*>(&n)) {
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

  assert(!"missed case in Traversal::visit");
}

void Traversal::visit(Not &n) {
  visit(static_cast<UnaryExpr&>(n));
}

void Traversal::visit(Number&) { }

void Traversal::visit(Or &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Quantifier &n) {
  visit(*n.var);
  visit(*n.step);
}

void Traversal::visit(Range &n) {
  visit(*n.min);
  visit(*n.max);
}

void Traversal::visit(Record &n) {
  for (VarDecl *f : n.fields)
    visit(*f);
}

void Traversal::visit(Ruleset &n) {
  for (Quantifier *q : n.quantifiers)
    visit(*q);
  for (Rule *r : n.rules)
    visit(*r);
}

void Traversal::visit(Scalarset &n) {
  visit(*n.bound);
}

void Traversal::visit(SimpleRule &n) {
  for (Quantifier *q : n.quantifiers)
    visit(*q);
  if (n.guard != nullptr)
    visit(*n.guard);
  for (Decl *d : n.decls)
    visit(*d);
  for (Stmt *s : n.body)
    visit(*s);
}

void Traversal::visit(StartState &n) {
  for (Quantifier *q : n.quantifiers)
    visit(*q);
  for (Decl *d : n.decls)
    visit(*d);
  for (Stmt *s : n.body)
    visit(*s);
}

void Traversal::visit(Sub &n) {
  visit(static_cast<BinaryExpr&>(n));
}

void Traversal::visit(Ternary &n) {
  visit(*n.cond);
  visit(*n.lhs);
  visit(*n.rhs);
}

void Traversal::visit(TypeDecl &n) {
  visit(*n.value);
}

void Traversal::visit(TypeExprID &n) {
  visit(*n.referent);
}

void Traversal::visit(UnaryExpr &n) {
  visit(*n.rhs);
}

void Traversal::visit(Undefine &n) {
  visit(*n.rhs);
}

void Traversal::visit(VarDecl &n) {
  visit(*n.type);
}

Traversal::~Traversal() { }

}
