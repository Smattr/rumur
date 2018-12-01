#include <cassert>
#include <cstddef>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/indexer.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>

namespace rumur {

void Indexer::visit(Add &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(AliasDecl &n) {
  n.unique_id = next++;
  dispatch(*n.value);
}

void Indexer::visit(AliasRule &n) {
  n.unique_id = next++;
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Indexer::visit(AliasStmt &n) {
  n.unique_id = next++;
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit(And &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Array &n) {
  n.unique_id = next++;
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void Indexer::visit(Assignment &n) {
  n.unique_id = next++;
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Indexer::visit_bexpr(BinaryExpr &n) {
  n.unique_id = next++;
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Indexer::visit(Clear &n) {
  n.unique_id = next++;
  dispatch(*n.rhs);
}

void Indexer::visit(ConstDecl &n) {
  n.unique_id = next++;
  dispatch(*n.value);
}

void Indexer::visit(Div &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Element &n) {
  n.unique_id = next++;
  dispatch(*n.array);
  dispatch(*n.index);
}

void Indexer::visit(Enum &n) {
  n.unique_id = next++;
}

void Indexer::visit(Eq &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(ErrorStmt &n) {
  n.unique_id = next++;
}

void Indexer::visit(Exists &n) {
  n.unique_id = next++;
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void Indexer::visit(ExprID &n) {
  n.unique_id = next++;
}

void Indexer::visit(Field &n) {
  n.unique_id = next++;
  dispatch(*n.record);
}

void Indexer::visit(For &n) {
  n.unique_id = next++;
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit(Forall &n) {
  n.unique_id = next++;
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void Indexer::visit(Function &n) {
  n.unique_id = next++;
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit(FunctionCall &n) {
  n.unique_id = next++;
  for (auto &a : n.arguments)
    dispatch(*a);
}

void Indexer::visit(Geq &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Gt &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(If &n) {
  n.unique_id = next++;
  for (IfClause &c : n.clauses)
    dispatch(c);
}

void Indexer::visit(IfClause &n) {
  n.unique_id = next++;
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit(Implication &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Leq &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Lt &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Mod &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Model &n) {
  n.unique_id = next++;
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Indexer::visit(Mul &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Negative &n) {
  visit_uexpr(static_cast<UnaryExpr&>(n));
}

void Indexer::visit(Neq &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Not &n) {
  visit_uexpr(static_cast<UnaryExpr&>(n));
}

void Indexer::visit(Number &n) {
  n.unique_id = next++;
}

void Indexer::visit(Or &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(ProcedureCall &n) {
  n.unique_id = next++;
  for (auto &a : n.arguments)
    dispatch(*a);
}

void Indexer::visit(Property &n) {
  n.unique_id = next++;
  dispatch(*n.expr);
}

void Indexer::visit(PropertyRule &n) {
  n.unique_id = next++;
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void Indexer::visit(PropertyStmt &n) {
  n.unique_id = next++;
  dispatch(n.property);
}

void Indexer::visit(Put &n) {
  n.unique_id = next++;
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Indexer::visit(Quantifier &n) {
  n.unique_id = next++;
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void Indexer::visit(Range &n) {
  n.unique_id = next++;
  dispatch(*n.min);
  dispatch(*n.max);
}

void Indexer::visit(Record &n) {
  n.unique_id = next++;
  for (auto &f : n.fields)
    dispatch(*f);
}

void Indexer::visit(Return &n) {
  n.unique_id = next++;
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Indexer::visit(Ruleset &n) {
  n.unique_id = next++;
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Indexer::visit(Scalarset &n) {
  n.unique_id = next++;
  dispatch(*n.bound);
}

void Indexer::visit(SimpleRule &n) {
  n.unique_id = next++;
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit(StartState &n) {
  n.unique_id = next++;
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit(Sub &n) {
  visit_bexpr(static_cast<BinaryExpr&>(n));
}

void Indexer::visit(Ternary &n) {
  n.unique_id = next++;
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Indexer::visit(TypeDecl &n) {
  n.unique_id = next++;
  dispatch(*n.value);
}

void Indexer::visit(TypeExprID &n) {
  n.unique_id = next++;
}

void Indexer::visit_uexpr(UnaryExpr &n) {
  n.unique_id = next++;
  dispatch(*n.rhs);
}

void Indexer::visit(Undefine &n) {
  n.unique_id = next++;
  dispatch(*n.rhs);
}

void Indexer::visit(VarDecl &n) {
  n.unique_id = next++;
  dispatch(*n.type);
}

void Indexer::visit(While &n) {
  n.unique_id = next++;
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

}
