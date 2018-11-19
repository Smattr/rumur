#pragma once

#include <cstddef>
#include <rumur/Expr.h>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>

namespace rumur {

class Indexer : public BaseTraversal {

 private:
  size_t next = 0;

 public:
  void visit(Add &n) final;
  void visit(AliasDecl &n) final;
  void visit(AliasRule &n) final;
  void visit(AliasStmt &n) final;
  void visit(And &n) final;
  void visit(Array &n) final;
  void visit(Assignment &n) final;
  void visit(Clear &n) final;
  void visit(ConstDecl &n) final;
  void visit(Div &n) final;
  void visit(Element &n) final;
  void visit(Enum&) final;
  void visit(Eq &n) final;
  void visit(ErrorStmt&) final;
  void visit(Exists &n) final;
  void visit(ExprID &n) final;
  void visit(Field &n) final;
  void visit(For &n) final;
  void visit(Forall &n) final;
  void visit(Function &n) final;
  void visit(FunctionCall &n) final;
  void visit(Geq &n) final;
  void visit(Gt &n) final;
  void visit(If &n) final;
  void visit(IfClause &n) final;
  void visit(Implication &n) final;
  void visit(Leq &n) final;
  void visit(Lt &n) final;
  void visit(Model &n) final;
  void visit(Mod &n) final;
  void visit(Mul &n) final;
  void visit(Negative &n) final;
  void visit(Neq &n) final;
  void visit(Not &n) final;
  void visit(Number &n) final;
  void visit(Or &n) final;
  void visit(ProcedureCall &n) final;
  void visit(Property &n) final;
  void visit(PropertyRule &n) final;
  void visit(PropertyStmt &n) final;
  void visit(Put &n) final;
  void visit(Quantifier &n) final;
  void visit(Range &n) final;
  void visit(Record &n) final;
  void visit(Return &n) final;
  void visit(Ruleset &n) final;
  void visit(Scalarset &n) final;
  void visit(SimpleRule &n) final;
  void visit(StartState &n) final;
  void visit(Sub &n) final;
  void visit(Ternary &n) final;
  void visit(TypeDecl &n) final;
  void visit(TypeExprID &n) final;
  void visit(Undefine &n) final;
  void visit(VarDecl &n) final;

  virtual ~Indexer() = default;

 private:
  void visit_bexpr(BinaryExpr &n);
  void visit_uexpr(UnaryExpr &n);
};

}
