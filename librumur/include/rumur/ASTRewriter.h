#pragma once

#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>

namespace rumur {

class ASTRewriter : public BaseTraversal<Node*> {

 public:
  Expr *visit_add(Add &n) override;
  Decl *visit_aliasdecl(AliasDecl &n) override;
  Rule *visit_aliasrule(AliasRule &n) override;
  Stmt *visit_aliasstmt(AliasStmt &n) override;
  Expr *visit_and(And &n) override;
  TypeExpr *visit_array(Array &n) override;
  Stmt *visit_assignment(Assignment &n) override;
  Stmt *visit_clear(Clear &n) override;
  Decl *visit_constdecl(ConstDecl &n) override;
  Expr *visit_div(Div &n) override;
  TypeExpr *visit_element(Element &n) override;
  TypeExpr *visit_enum(Enum &n) override;
  Expr *visit_eq(Eq &n) override;
  Stmt *visit_errorstmt(ErrorStmt &n) override;
  Expr *visit_exists(Exists &n) override;
  Expr *visit_exprid(ExprID &n) override;
  Field *visit_field(Field &n) override;
  Stmt *visit_for(For &n) override;
  Expr *visit_forall(Forall &n) override;
  Function *visit_function(Function &n) override;
  Expr *visit_functioncall(FunctionCall &n) override;
  Expr *visit_geq(Geq &n) override;
  Expr *visit_gt(Gt &n) override;
  Stmt *visit_if(If &n) override;
  IfClause *visit_ifclause(IfClause &n) override;
  Expr *visit_implication(Implication &n) override;
  Expr *visit_isundefined(IsUndefined &n) override;
  Expr *visit_leq(Leq &n) override;
  Expr *visit_lt(Lt &n) override;
  Model *visit_model(Model &n) override;
  Expr *visit_mod(Mod &n) override;
  Expr *visit_mul(Mul &n) override;
  Expr *visit_negative(Negative &n) override;
  Expr *visit_neq(Neq &n) override;
  Expr *visit_not(Not &n) override;
  Expr *visit_number(Number &n) override;
  Expr *visit_or(Or &n) override;
  Stmt *visit_procedurecall(ProcedureCall &n) override;
  Property *visit_property(Property &n) override;
  Rule *visit_propertyrule(PropertyRule &n) override;
  Stmt *visit_propertystmt(PropertyStmt &n) override;
  Stmt *visit_put(Put &n) override;
  Quantifier *visit_quantifier(Quantifier &n) override;
  TypeExpr *visit_range(Range &n) override;
  TypeExpr *visit_record(Record &n) override;
  Stmt *visit_return(Return &n) override;
  Rule *visit_ruleset(Ruleset &n) override;
  TypeExpr *visit_scalarset(Scalarset &n) override;
  Rule *visit_simplerule(SimpleRule &n) override;
  Rule *visit_startstate(StartState &n) override;
  Expr *visit_sub(Sub &n) override;
  Stmt *visit_switch(Switch &n) override;
  SwitchCase *visit_switchcase(SwitchCase &n) override;
  Expr *visit_ternary(Ternary &n) override;
  Decl *visit_typedecl(TypeDecl &n) override;
  TypeExpr *visit_typeexprid(TypeExprID &n) override;
  Stmt *visit_undefine(Undefine &n) override;
  Decl *visit_vardecl(VarDecl &n) override;
  Stmt *visit_while(While &n) override;

  // force class to be abstract
  virtual ~ASTRewriter() = 0;

 private:
  Expr *visit_bexpr(BinaryExpr &n);
  Expr *visit_uexpr(UnaryExpr &n);
};

}
