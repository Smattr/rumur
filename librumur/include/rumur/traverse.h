#pragma once

#include <cstddef>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

/* Generic abstract syntax tree traversal interface with no implementation. If
 * you want to implement a traversal that handles every Node type you should
 * define a class that inherits from this. If you want a more liberal base class
 * that provides default implementations for the 'visit' methods you don't need
 * to override, inherit from Traversal below.
 */
class RUMUR_API_WITH_RTTI BaseTraversal {

public:
  virtual void visit_add(Add &n) = 0;
  virtual void visit_aliasdecl(AliasDecl &n) = 0;
  virtual void visit_aliasrule(AliasRule &n) = 0;
  virtual void visit_aliasstmt(AliasStmt &n) = 0;
  virtual void visit_and(And &n) = 0;
  virtual void visit_array(Array &n) = 0;
  virtual void visit_assignment(Assignment &n) = 0;
  virtual void visit_band(Band &n) = 0;
  virtual void visit_bnot(Bnot &n) = 0;
  virtual void visit_bor(Bor &n) = 0;
  virtual void visit_clear(Clear &n) = 0;
  virtual void visit_constdecl(ConstDecl &n) = 0;
  virtual void visit_div(Div &n) = 0;
  virtual void visit_element(Element &n) = 0;
  virtual void visit_enum(Enum &n) = 0;
  virtual void visit_eq(Eq &n) = 0;
  virtual void visit_errorstmt(ErrorStmt &n) = 0;
  virtual void visit_exists(Exists &n) = 0;
  virtual void visit_exprid(ExprID &n) = 0;
  virtual void visit_field(Field &n) = 0;
  virtual void visit_for(For &n) = 0;
  virtual void visit_forall(Forall &n) = 0;
  virtual void visit_function(Function &n) = 0;
  virtual void visit_functioncall(FunctionCall &n) = 0;
  virtual void visit_geq(Geq &n) = 0;
  virtual void visit_gt(Gt &n) = 0;
  virtual void visit_if(If &n) = 0;
  virtual void visit_ifclause(IfClause &n) = 0;
  virtual void visit_implication(Implication &n) = 0;
  virtual void visit_isundefined(IsUndefined &n) = 0;
  virtual void visit_leq(Leq &n) = 0;
  virtual void visit_lsh(Lsh &n) = 0;
  virtual void visit_lt(Lt &n) = 0;
  virtual void visit_model(Model &n) = 0;
  virtual void visit_mod(Mod &n) = 0;
  virtual void visit_mul(Mul &n) = 0;
  virtual void visit_negative(Negative &n) = 0;
  virtual void visit_neq(Neq &n) = 0;
  virtual void visit_not(Not &n) = 0;
  virtual void visit_number(Number &n) = 0;
  virtual void visit_or(Or &n) = 0;
  virtual void visit_procedurecall(ProcedureCall &n) = 0;
  virtual void visit_property(Property &n) = 0;
  virtual void visit_propertyrule(PropertyRule &n) = 0;
  virtual void visit_propertystmt(PropertyStmt &n) = 0;
  virtual void visit_put(Put &n) = 0;
  virtual void visit_quantifier(Quantifier &n) = 0;
  virtual void visit_range(Range &n) = 0;
  virtual void visit_record(Record &n) = 0;
  virtual void visit_return(Return &n) = 0;
  virtual void visit_rsh(Rsh &n) = 0;
  virtual void visit_ruleset(Ruleset &n) = 0;
  virtual void visit_scalarset(Scalarset &n) = 0;
  virtual void visit_simplerule(SimpleRule &n) = 0;
  virtual void visit_startstate(StartState &n) = 0;
  virtual void visit_sub(Sub &n) = 0;
  virtual void visit_switch(Switch &n) = 0;
  virtual void visit_switchcase(SwitchCase &n) = 0;
  virtual void visit_ternary(Ternary &n) = 0;
  virtual void visit_typedecl(TypeDecl &n) = 0;
  virtual void visit_typeexprid(TypeExprID &n) = 0;
  virtual void visit_undefine(Undefine &n) = 0;
  virtual void visit_vardecl(VarDecl &n) = 0;
  virtual void visit_while(While &n) = 0;
  virtual void visit_xor(Xor &n) = 0;

  /* Visitation dispatch. This simply determines the type of the Node argument
   * and calls the appropriate specialised 'visit' method. This is not virtual
   * because we do not anticipate use cases where this behaviour needs to
   * change.
   */
  void dispatch(Node &n);

  // Unlike the other visitation methods, we provide an implementation for
  // the ambiguous nodes because they only exist in an unresolved AST. We assume
  // that inheritors, even if they want to handle “everything,” may not want to
  // handle these synthetic types.
  virtual void visit_ambiguousamp(AmbiguousAmp &n);
  virtual void visit_ambiguouspipe(AmbiguousPipe &n);

  virtual ~BaseTraversal() = default;
};

class RUMUR_API_WITH_RTTI Traversal : public BaseTraversal {

public:
  void visit_add(Add &n) override;
  void visit_aliasdecl(AliasDecl &n) override;
  void visit_aliasrule(AliasRule &n) override;
  void visit_aliasstmt(AliasStmt &n) override;
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

  // Force class to be abstract
  virtual ~Traversal() = 0;

private:
  void visit_bexpr(BinaryExpr &n);
  void visit_uexpr(UnaryExpr &n);
};

// Read-only equivalent of BaseTraversal.
class RUMUR_API_WITH_RTTI ConstBaseTraversal {

public:
  virtual void visit_add(const Add &n) = 0;
  virtual void visit_aliasdecl(const AliasDecl &n) = 0;
  virtual void visit_aliasrule(const AliasRule &n) = 0;
  virtual void visit_aliasstmt(const AliasStmt &n) = 0;
  virtual void visit_and(const And &n) = 0;
  virtual void visit_array(const Array &n) = 0;
  virtual void visit_assignment(const Assignment &n) = 0;
  virtual void visit_band(const Band &n) = 0;
  virtual void visit_bnot(const Bnot &n) = 0;
  virtual void visit_bor(const Bor &n) = 0;
  virtual void visit_clear(const Clear &n) = 0;
  virtual void visit_constdecl(const ConstDecl &n) = 0;
  virtual void visit_div(const Div &n) = 0;
  virtual void visit_element(const Element &n) = 0;
  virtual void visit_enum(const Enum &n) = 0;
  virtual void visit_eq(const Eq &n) = 0;
  virtual void visit_errorstmt(const ErrorStmt &n) = 0;
  virtual void visit_exists(const Exists &n) = 0;
  virtual void visit_exprid(const ExprID &n) = 0;
  virtual void visit_field(const Field &n) = 0;
  virtual void visit_for(const For &n) = 0;
  virtual void visit_forall(const Forall &n) = 0;
  virtual void visit_function(const Function &n) = 0;
  virtual void visit_functioncall(const FunctionCall &n) = 0;
  virtual void visit_geq(const Geq &n) = 0;
  virtual void visit_gt(const Gt &n) = 0;
  virtual void visit_if(const If &n) = 0;
  virtual void visit_ifclause(const IfClause &n) = 0;
  virtual void visit_implication(const Implication &n) = 0;
  virtual void visit_isundefined(const IsUndefined &n) = 0;
  virtual void visit_leq(const Leq &n) = 0;
  virtual void visit_lsh(const Lsh &n) = 0;
  virtual void visit_lt(const Lt &n) = 0;
  virtual void visit_model(const Model &n) = 0;
  virtual void visit_mod(const Mod &n) = 0;
  virtual void visit_mul(const Mul &n) = 0;
  virtual void visit_negative(const Negative &n) = 0;
  virtual void visit_neq(const Neq &n) = 0;
  virtual void visit_not(const Not &n) = 0;
  virtual void visit_number(const Number &n) = 0;
  virtual void visit_or(const Or &n) = 0;
  virtual void visit_procedurecall(const ProcedureCall &n) = 0;
  virtual void visit_property(const Property &n) = 0;
  virtual void visit_propertyrule(const PropertyRule &n) = 0;
  virtual void visit_propertystmt(const PropertyStmt &n) = 0;
  virtual void visit_put(const Put &n) = 0;
  virtual void visit_quantifier(const Quantifier &n) = 0;
  virtual void visit_range(const Range &n) = 0;
  virtual void visit_record(const Record &n) = 0;
  virtual void visit_return(const Return &n) = 0;
  virtual void visit_rsh(const Rsh &n) = 0;
  virtual void visit_ruleset(const Ruleset &n) = 0;
  virtual void visit_scalarset(const Scalarset &n) = 0;
  virtual void visit_simplerule(const SimpleRule &n) = 0;
  virtual void visit_startstate(const StartState &n) = 0;
  virtual void visit_sub(const Sub &n) = 0;
  virtual void visit_switch(const Switch &n) = 0;
  virtual void visit_switchcase(const SwitchCase &n) = 0;
  virtual void visit_ternary(const Ternary &n) = 0;
  virtual void visit_typedecl(const TypeDecl &n) = 0;
  virtual void visit_typeexprid(const TypeExprID &n) = 0;
  virtual void visit_undefine(const Undefine &n) = 0;
  virtual void visit_vardecl(const VarDecl &n) = 0;
  virtual void visit_while(const While &n) = 0;
  virtual void visit_xor(const Xor &n) = 0;

  void dispatch(const Node &n);

  virtual void visit_ambiguousamp(const AmbiguousAmp &n);
  virtual void visit_ambiguouspipe(const AmbiguousPipe &n);

  virtual ~ConstBaseTraversal() = default;
};

// Read-only equivalent of Traversal.
class RUMUR_API_WITH_RTTI ConstTraversal : public ConstBaseTraversal {

public:
  void visit_add(const Add &n) override;
  void visit_aliasdecl(const AliasDecl &n) override;
  void visit_aliasrule(const AliasRule &n) override;
  void visit_aliasstmt(const AliasStmt &n) override;
  void visit_and(const And &n) override;
  void visit_array(const Array &n) override;
  void visit_assignment(const Assignment &n) override;
  void visit_band(const Band &n) override;
  void visit_bnot(const Bnot &n) override;
  void visit_bor(const Bor &n) override;
  void visit_clear(const Clear &n) override;
  void visit_constdecl(const ConstDecl &n) override;
  void visit_div(const Div &n) override;
  void visit_element(const Element &n) override;
  void visit_enum(const Enum &n) override;
  void visit_eq(const Eq &n) override;
  void visit_errorstmt(const ErrorStmt &n) override;
  void visit_exists(const Exists &n) override;
  void visit_exprid(const ExprID &n) override;
  void visit_field(const Field &n) override;
  void visit_for(const For &n) override;
  void visit_forall(const Forall &n) override;
  void visit_function(const Function &n) override;
  void visit_functioncall(const FunctionCall &n) override;
  void visit_geq(const Geq &n) override;
  void visit_gt(const Gt &n) override;
  void visit_if(const If &n) override;
  void visit_ifclause(const IfClause &n) override;
  void visit_implication(const Implication &n) override;
  void visit_isundefined(const IsUndefined &n) override;
  void visit_leq(const Leq &n) override;
  void visit_lsh(const Lsh &n) override;
  void visit_lt(const Lt &n) override;
  void visit_model(const Model &n) override;
  void visit_mod(const Mod &n) override;
  void visit_mul(const Mul &n) override;
  void visit_negative(const Negative &n) override;
  void visit_neq(const Neq &n) override;
  void visit_not(const Not &n) override;
  void visit_number(const Number &n) override;
  void visit_or(const Or &n) override;
  void visit_procedurecall(const ProcedureCall &n) override;
  void visit_property(const Property &n) override;
  void visit_propertyrule(const PropertyRule &n) override;
  void visit_propertystmt(const PropertyStmt &n) override;
  void visit_put(const Put &n) override;
  void visit_quantifier(const Quantifier &n) override;
  void visit_range(const Range &n) override;
  void visit_record(const Record &n) override;
  void visit_return(const Return &n) override;
  void visit_rsh(const Rsh &n) override;
  void visit_ruleset(const Ruleset &n) override;
  void visit_scalarset(const Scalarset &n) override;
  void visit_simplerule(const SimpleRule &n) override;
  void visit_startstate(const StartState &n) override;
  void visit_sub(const Sub &n) override;
  void visit_switch(const Switch &n) override;
  void visit_switchcase(const SwitchCase &n) override;
  void visit_ternary(const Ternary &n) override;
  void visit_typedecl(const TypeDecl &n) override;
  void visit_typeexprid(const TypeExprID &n) override;
  void visit_undefine(const Undefine &n) override;
  void visit_vardecl(const VarDecl &n) override;
  void visit_while(const While &n) override;
  void visit_xor(const Xor &n) override;

  // Force class to be abstract
  virtual ~ConstTraversal() = 0;

private:
  void visit_bexpr(const BinaryExpr &n);
  void visit_uexpr(const UnaryExpr &n);
};

/* Generic base for read-only traversals that only need to act on expressions.
 * This gives you a default implementation for visitation of any non-expression
 * node.
 */
class RUMUR_API_WITH_RTTI ConstExprTraversal : public ConstBaseTraversal {

public:
  void visit_aliasdecl(const AliasDecl &n) final;
  void visit_aliasrule(const AliasRule &n) final;
  void visit_aliasstmt(const AliasStmt &n) final;
  void visit_array(const Array &n) final;
  void visit_assignment(const Assignment &n) final;
  void visit_clear(const Clear &n) final;
  void visit_constdecl(const ConstDecl &n) final;
  void visit_enum(const Enum &n) final;
  void visit_errorstmt(const ErrorStmt &n) final;
  void visit_for(const For &n) final;
  void visit_function(const Function &n) final;
  void visit_if(const If &n) final;
  void visit_ifclause(const IfClause &n) final;
  void visit_model(const Model &n) final;
  void visit_procedurecall(const ProcedureCall &n) final;
  void visit_property(const Property &n) final;
  void visit_propertyrule(const PropertyRule &n) final;
  void visit_propertystmt(const PropertyStmt &n) final;
  void visit_put(const Put &n) final;
  void visit_quantifier(const Quantifier &n) final;
  void visit_range(const Range &n) final;
  void visit_record(const Record &n) final;
  void visit_return(const Return &n) final;
  void visit_ruleset(const Ruleset &n) final;
  void visit_scalarset(const Scalarset &n) final;
  void visit_simplerule(const SimpleRule &n) final;
  void visit_startstate(const StartState &n) final;
  void visit_switch(const Switch &n) final;
  void visit_switchcase(const SwitchCase &n) final;
  void visit_typedecl(const TypeDecl &n) final;
  void visit_typeexprid(const TypeExprID &n) final;
  void visit_undefine(const Undefine &n) final;
  void visit_vardecl(const VarDecl &n) final;
  void visit_while(const While &n) final;

  virtual ~ConstExprTraversal() = default;
};

/* Generic base for read-only traversals that only need to act on statements.
 * This gives you a default implementation for visitation of any non-statement
 * node.
 */
class RUMUR_API_WITH_RTTI ConstStmtTraversal : public ConstBaseTraversal {

public:
  void visit_add(const Add &n) final;
  void visit_aliasdecl(const AliasDecl &n) final;
  void visit_aliasrule(const AliasRule &n) final;
  void visit_and(const And &n) final;
  void visit_array(const Array &n) final;
  void visit_band(const Band &n) final;
  void visit_bnot(const Bnot &n) final;
  void visit_bor(const Bor &n) final;
  void visit_constdecl(const ConstDecl &n) final;
  void visit_div(const Div &n) final;
  void visit_element(const Element &n) final;
  void visit_enum(const Enum &n) final;
  void visit_eq(const Eq &n) final;
  void visit_exists(const Exists &n) final;
  void visit_exprid(const ExprID &n) final;
  void visit_field(const Field &n) final;
  void visit_forall(const Forall &n) final;
  void visit_function(const Function &n) final;
  void visit_functioncall(const FunctionCall &n) final;
  void visit_geq(const Geq &n) final;
  void visit_gt(const Gt &n) final;
  void visit_ifclause(const IfClause &n) final;
  void visit_implication(const Implication &n) final;
  void visit_isundefined(const IsUndefined &n) final;
  void visit_leq(const Leq &n) final;
  void visit_lsh(const Lsh &n) final;
  void visit_lt(const Lt &n) final;
  void visit_model(const Model &n) final;
  void visit_mod(const Mod &n) final;
  void visit_mul(const Mul &n) final;
  void visit_negative(const Negative &n) final;
  void visit_neq(const Neq &n) final;
  void visit_not(const Not &n) final;
  void visit_number(const Number &n) final;
  void visit_or(const Or &n) final;
  void visit_property(const Property &n) final;
  void visit_propertyrule(const PropertyRule &n) final;
  void visit_quantifier(const Quantifier &n) final;
  void visit_range(const Range &n) final;
  void visit_record(const Record &n) final;
  void visit_rsh(const Rsh &n) final;
  void visit_ruleset(const Ruleset &n) final;
  void visit_scalarset(const Scalarset &n) final;
  void visit_simplerule(const SimpleRule &n) final;
  void visit_startstate(const StartState &n) final;
  void visit_sub(const Sub &n) final;
  void visit_switchcase(const SwitchCase &n) final;
  void visit_ternary(const Ternary &n) final;
  void visit_typedecl(const TypeDecl &n) final;
  void visit_typeexprid(const TypeExprID &n) final;
  void visit_vardecl(const VarDecl &n) final;
  void visit_xor(const Xor &n) final;

  virtual ~ConstStmtTraversal() = default;

private:
  void visit_bexpr(const BinaryExpr &n);
  void visit_uexpr(const UnaryExpr &n);
};

// Generic base for read-only traversals that only need to act on TypeExprs
class RUMUR_API_WITH_RTTI ConstTypeTraversal : public ConstBaseTraversal {

public:
  void visit_add(const Add &n) final;
  void visit_aliasdecl(const AliasDecl &n) final;
  void visit_aliasrule(const AliasRule &n) final;
  void visit_aliasstmt(const AliasStmt &n) final;
  void visit_and(const And &n) final;
  void visit_assignment(const Assignment &n) final;
  void visit_band(const Band &n) final;
  void visit_bnot(const Bnot &n) final;
  void visit_bor(const Bor &n) final;
  void visit_clear(const Clear &n) final;
  void visit_constdecl(const ConstDecl &n) final;
  void visit_div(const Div &n) final;
  void visit_element(const Element &n) final;
  void visit_eq(const Eq &n) final;
  void visit_errorstmt(const ErrorStmt &n) final;
  void visit_exists(const Exists &n) final;
  void visit_exprid(const ExprID &n) final;
  void visit_field(const Field &n) final;
  void visit_for(const For &n) final;
  void visit_forall(const Forall &n) final;
  void visit_function(const Function &n) final;
  void visit_functioncall(const FunctionCall &n) final;
  void visit_geq(const Geq &n) final;
  void visit_gt(const Gt &n) final;
  void visit_if(const If &n) final;
  void visit_ifclause(const IfClause &n) final;
  void visit_implication(const Implication &n) final;
  void visit_isundefined(const IsUndefined &n) final;
  void visit_leq(const Leq &n) final;
  void visit_lsh(const Lsh &n) final;
  void visit_lt(const Lt &n) final;
  void visit_model(const Model &n) final;
  void visit_mod(const Mod &n) final;
  void visit_mul(const Mul &n) final;
  void visit_negative(const Negative &n) final;
  void visit_neq(const Neq &n) final;
  void visit_not(const Not &n) final;
  void visit_number(const Number &n) final;
  void visit_or(const Or &n) final;
  void visit_procedurecall(const ProcedureCall &n) final;
  void visit_property(const Property &n) final;
  void visit_propertyrule(const PropertyRule &n) final;
  void visit_propertystmt(const PropertyStmt &n) final;
  void visit_put(const Put &n) final;
  void visit_quantifier(const Quantifier &n) final;
  void visit_return(const Return &n) final;
  void visit_rsh(const Rsh &n) final;
  void visit_ruleset(const Ruleset &n) final;
  void visit_simplerule(const SimpleRule &n) final;
  void visit_startstate(const StartState &n) final;
  void visit_sub(const Sub &n) final;
  void visit_switch(const Switch &n) final;
  void visit_switchcase(const SwitchCase &n) final;
  void visit_ternary(const Ternary &n) final;
  void visit_typedecl(const TypeDecl &n) final;
  void visit_undefine(const Undefine &n) final;
  void visit_vardecl(const VarDecl &n) final;
  void visit_while(const While &n) final;
  void visit_xor(const Xor &n) final;

  virtual ~ConstTypeTraversal() = default;

private:
  void visit_bexpr(const BinaryExpr &n);
  void visit_uexpr(const UnaryExpr &n);
};
} // namespace rumur
