#pragma once

#include <cstddef>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <rumur/traverse.h>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

class RUMUR_API_WITH_RTTI Indexer : public BaseTraversal {

private:
  size_t next = 0;

public:
  void visit_add(Add &n) override;
  void visit_aliasdecl(AliasDecl &n) override;
  void visit_aliasrule(AliasRule &n) override;
  void visit_aliasstmt(AliasStmt &n) override;
  void visit_ambiguousamp(AmbiguousAmp &n) override;
  void visit_ambiguouspipe(AmbiguousPipe &n) override;
  void visit_and(And &n) override;
  void visit_array(Array &n) override;
  void visit_assignment(Assignment &n) override;
  void visit_band(Band &n) override;
  void visit_bnot(Bnot &n) override;
  void visit_bor(Bor &n) override;
  void visit_clear(Clear &n) override;
  void visit_constdecl(ConstDecl &n) override;
  void visit_div(Div &n) override;
  void visit_element(Element &n) override;
  void visit_enum(Enum &n) override;
  void visit_eq(Eq &n) override;
  void visit_errorstmt(ErrorStmt &n) override;
  void visit_exists(Exists &n) override;
  void visit_exprid(ExprID &n) override;
  void visit_field(Field &n) override;
  void visit_for(For &n) override;
  void visit_forall(Forall &n) override;
  void visit_function(Function &n) override;
  void visit_functioncall(FunctionCall &n) override;
  void visit_geq(Geq &n) override;
  void visit_gt(Gt &n) override;
  void visit_if(If &n) override;
  void visit_ifclause(IfClause &n) override;
  void visit_implication(Implication &n) override;
  void visit_isundefined(IsUndefined &n) override;
  void visit_leq(Leq &n) override;
  void visit_lsh(Lsh &n) override;
  void visit_lt(Lt &n) override;
  void visit_model(Model &n) override;
  void visit_mod(Mod &n) override;
  void visit_mul(Mul &n) override;
  void visit_negative(Negative &n) override;
  void visit_neq(Neq &n) override;
  void visit_not(Not &n) override;
  void visit_number(Number &n) override;
  void visit_or(Or &n) override;
  void visit_procedurecall(ProcedureCall &n) override;
  void visit_property(Property &n) override;
  void visit_propertyrule(PropertyRule &n) override;
  void visit_propertystmt(PropertyStmt &n) override;
  void visit_put(Put &n) override;
  void visit_quantifier(Quantifier &n) override;
  void visit_range(Range &n) override;
  void visit_record(Record &n) override;
  void visit_return(Return &n) override;
  void visit_rsh(Rsh &n) override;
  void visit_ruleset(Ruleset &n) override;
  void visit_scalarset(Scalarset &n) override;
  void visit_simplerule(SimpleRule &n) override;
  void visit_startstate(StartState &n) override;
  void visit_sub(Sub &n) override;
  void visit_switch(Switch &n) override;
  void visit_switchcase(SwitchCase &n) override;
  void visit_ternary(Ternary &n) override;
  void visit_typedecl(TypeDecl &n) override;
  void visit_typeexprid(TypeExprID &n) override;
  void visit_undefine(Undefine &n) override;
  void visit_vardecl(VarDecl &n) override;
  void visit_while(While &n) override;
  void visit_xor(Xor &n) override;

  virtual ~Indexer() = default;

private:
  void visit_bexpr(BinaryExpr &n);
  void visit_uexpr(UnaryExpr &n);
};

} // namespace rumur
