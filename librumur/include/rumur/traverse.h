#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <typeinfo>

namespace rumur {

/* Generic abstract syntax tree traversal interface with no implementation. If
 * you want to implement a traversal that handles every Node type you should
 * define a class that inherits from this. If you want a more liberal base class
 * that provides default implementations for the 'visit' methods you don't need
 * to override, inherit from Traversal below.
 */
template<typename RETURN_TYPE = void>
class BaseTraversal {

 public:
  virtual RETURN_TYPE visit_add(Add &n) = 0;
  virtual RETURN_TYPE visit_aliasdecl(AliasDecl &n) = 0;
  virtual RETURN_TYPE visit_aliasrule(AliasRule &n) = 0;
  virtual RETURN_TYPE visit_aliasstmt(AliasStmt &n) = 0;
  virtual RETURN_TYPE visit_and(And &n) = 0;
  virtual RETURN_TYPE visit_array(Array &n) = 0;
  virtual RETURN_TYPE visit_assignment(Assignment &n) = 0;
  virtual RETURN_TYPE visit_clear(Clear &n) = 0;
  virtual RETURN_TYPE visit_constdecl(ConstDecl &n) = 0;
  virtual RETURN_TYPE visit_div(Div &n) = 0;
  virtual RETURN_TYPE visit_element(Element &n) = 0;
  virtual RETURN_TYPE visit_enum(Enum &n) = 0;
  virtual RETURN_TYPE visit_eq(Eq &n) = 0;
  virtual RETURN_TYPE visit_errorstmt(ErrorStmt &n) = 0;
  virtual RETURN_TYPE visit_exists(Exists &n) = 0;
  virtual RETURN_TYPE visit_exprid(ExprID &n) = 0;
  virtual RETURN_TYPE visit_field(Field &n) = 0;
  virtual RETURN_TYPE visit_for(For &n) = 0;
  virtual RETURN_TYPE visit_forall(Forall &n) = 0;
  virtual RETURN_TYPE visit_function(Function &n) = 0;
  virtual RETURN_TYPE visit_functioncall(FunctionCall &n) = 0;
  virtual RETURN_TYPE visit_geq(Geq &n) = 0;
  virtual RETURN_TYPE visit_gt(Gt &n) = 0;
  virtual RETURN_TYPE visit_if(If &n) = 0;
  virtual RETURN_TYPE visit_ifclause(IfClause &n) = 0;
  virtual RETURN_TYPE visit_implication(Implication &n) = 0;
  virtual RETURN_TYPE visit_isundefined(IsUndefined &n) = 0;
  virtual RETURN_TYPE visit_leq(Leq &n) = 0;
  virtual RETURN_TYPE visit_lt(Lt &n) = 0;
  virtual RETURN_TYPE visit_model(Model &n) = 0;
  virtual RETURN_TYPE visit_mod(Mod &n) = 0;
  virtual RETURN_TYPE visit_mul(Mul &n) = 0;
  virtual RETURN_TYPE visit_negative(Negative &n) = 0;
  virtual RETURN_TYPE visit_neq(Neq &n) = 0;
  virtual RETURN_TYPE visit_not(Not &n) = 0;
  virtual RETURN_TYPE visit_number(Number &n) = 0;
  virtual RETURN_TYPE visit_or(Or &n) = 0;
  virtual RETURN_TYPE visit_procedurecall(ProcedureCall &n) = 0;
  virtual RETURN_TYPE visit_property(Property &n) = 0;
  virtual RETURN_TYPE visit_propertyrule(PropertyRule &n) = 0;
  virtual RETURN_TYPE visit_propertystmt(PropertyStmt &n) = 0;
  virtual RETURN_TYPE visit_put(Put &n) = 0;
  virtual RETURN_TYPE visit_quantifier(Quantifier &n) = 0;
  virtual RETURN_TYPE visit_range(Range &n) = 0;
  virtual RETURN_TYPE visit_record(Record &n) = 0;
  virtual RETURN_TYPE visit_return(Return &n) = 0;
  virtual RETURN_TYPE visit_ruleset(Ruleset &n) = 0;
  virtual RETURN_TYPE visit_scalarset(Scalarset &n) = 0;
  virtual RETURN_TYPE visit_simplerule(SimpleRule &n) = 0;
  virtual RETURN_TYPE visit_startstate(StartState &n) = 0;
  virtual RETURN_TYPE visit_sub(Sub &n) = 0;
  virtual RETURN_TYPE visit_switch(Switch &n) = 0;
  virtual RETURN_TYPE visit_switchcase(SwitchCase &n) = 0;
  virtual RETURN_TYPE visit_ternary(Ternary &n) = 0;
  virtual RETURN_TYPE visit_typedecl(TypeDecl &n) = 0;
  virtual RETURN_TYPE visit_typeexprid(TypeExprID &n) = 0;
  virtual RETURN_TYPE visit_undefine(Undefine &n) = 0;
  virtual RETURN_TYPE visit_vardecl(VarDecl &n) = 0;
  virtual RETURN_TYPE visit_while(While &n) = 0;

  /* Visitation dispatch. This simply determines the type of the Node argument
   * and calls the appropriate specialised 'visit' method. This is not virtual
   * because we do not anticipate use cases where this behaviour needs to
   * change.
   */
  RETURN_TYPE dispatch(Node &n) {

    if (auto i = dynamic_cast<Add*>(&n)) {
      return visit_add(*i);
    }

    if (auto i = dynamic_cast<AliasDecl*>(&n)) {
      return visit_aliasdecl(*i);
    }

    if (auto i = dynamic_cast<AliasRule*>(&n)) {
      return visit_aliasrule(*i);
    }

    if (auto i = dynamic_cast<AliasStmt*>(&n)) {
      return visit_aliasstmt(*i);
    }

    if (auto i = dynamic_cast<And*>(&n)) {
      return visit_and(*i);
    }

    if (auto i = dynamic_cast<Array*>(&n)) {
      return visit_array(*i);
    }

    if (auto i = dynamic_cast<Assignment*>(&n)) {
      return visit_assignment(*i);
    }

    if (auto i = dynamic_cast<ConstDecl*>(&n)) {
      return visit_constdecl(*i);
    }

    if (auto i = dynamic_cast<Clear*>(&n)) {
      return visit_clear(*i);
    }

    if (auto i = dynamic_cast<Div*>(&n)) {
      return visit_div(*i);
    }

    if (auto i = dynamic_cast<Element*>(&n)) {
      return visit_element(*i);
    }

    if (auto i = dynamic_cast<Function*>(&n)) {
      return visit_function(*i);
    }

    if (auto i = dynamic_cast<FunctionCall*>(&n)) {
      return visit_functioncall(*i);
    }

    if (auto i = dynamic_cast<Enum*>(&n)) {
      return visit_enum(*i);
    }

    if (auto i = dynamic_cast<Eq*>(&n)) {
      return visit_eq(*i);
    }

    if (auto i = dynamic_cast<ErrorStmt*>(&n)) {
      return visit_errorstmt(*i);
    }

    if (auto i = dynamic_cast<Exists*>(&n)) {
      return visit_exists(*i);
    }

    if (auto i = dynamic_cast<ExprID*>(&n)) {
      return visit_exprid(*i);
    }

    if (auto i = dynamic_cast<Field*>(&n)) {
      return visit_field(*i);
    }

    if (auto i = dynamic_cast<For*>(&n)) {
      return visit_for(*i);
    }

    if (auto i = dynamic_cast<Forall*>(&n)) {
      return visit_forall(*i);
    }

    if (auto i = dynamic_cast<Geq*>(&n)) {
      return visit_geq(*i);
    }

    if (auto i = dynamic_cast<Gt*>(&n)) {
      return visit_gt(*i);
    }

    if (auto i = dynamic_cast<If*>(&n)) {
      return visit_if(*i);
    }

    if (auto i = dynamic_cast<IfClause*>(&n)) {
      return visit_ifclause(*i);
    }

    if (auto i = dynamic_cast<Implication*>(&n)) {
      return visit_implication(*i);
    }

    if (auto i = dynamic_cast<IsUndefined*>(&n)) {
      return visit_isundefined(*i);
    }

    if (auto i = dynamic_cast<Leq*>(&n)) {
      return visit_leq(*i);
    }

    if (auto i = dynamic_cast<Lt*>(&n)) {
      return visit_lt(*i);
    }

    if (auto i = dynamic_cast<Model*>(&n)) {
      return visit_model(*i);
    }

    if (auto i = dynamic_cast<Mod*>(&n)) {
      return visit_mod(*i);
    }

    if (auto i = dynamic_cast<Mul*>(&n)) {
      return visit_mul(*i);
    }

    if (auto i = dynamic_cast<Negative*>(&n)) {
      return visit_negative(*i);
    }

    if (auto i = dynamic_cast<Neq*>(&n)) {
      return visit_neq(*i);
    }

    if (auto i = dynamic_cast<Not*>(&n)) {
      return visit_not(*i);
    }

    if (auto i = dynamic_cast<Number*>(&n)) {
      return visit_number(*i);
    }

    if (auto i = dynamic_cast<Or*>(&n)) {
      return visit_or(*i);
    }

    if (auto i = dynamic_cast<ProcedureCall*>(&n)) {
      return visit_procedurecall(*i);
    }

    if (auto i = dynamic_cast<Property*>(&n)) {
      return visit_property(*i);
    }

    if (auto i = dynamic_cast<PropertyRule*>(&n)) {
      return visit_propertyrule(*i);
    }

    if (auto i = dynamic_cast<PropertyStmt*>(&n)) {
      return visit_propertystmt(*i);
    }

    if (auto i = dynamic_cast<Put*>(&n)) {
      return visit_put(*i);
    }

    if (auto i = dynamic_cast<Quantifier*>(&n)) {
      return visit_quantifier(*i);
    }

    if (auto i = dynamic_cast<Range*>(&n)) {
      return visit_range(*i);
    }

    if (auto i = dynamic_cast<Record*>(&n)) {
      return visit_record(*i);
    }

    if (auto i = dynamic_cast<Return*>(&n)) {
      return visit_return(*i);
    }

    if (auto i = dynamic_cast<Ruleset*>(&n)) {
      return visit_ruleset(*i);
    }

    if (auto i = dynamic_cast<Scalarset*>(&n)) {
      return visit_scalarset(*i);
    }

    if (auto i = dynamic_cast<SimpleRule*>(&n)) {
      return visit_simplerule(*i);
    }

    if (auto i = dynamic_cast<StartState*>(&n)) {
      return visit_startstate(*i);
    }

    if (auto i = dynamic_cast<Sub*>(&n)) {
      return visit_sub(*i);
    }

    if (auto i = dynamic_cast<Switch*>(&n)) {
      return visit_switch(*i);
    }

    if (auto i = dynamic_cast<SwitchCase*>(&n)) {
      return visit_switchcase(*i);
    }

    if (auto i = dynamic_cast<Ternary*>(&n)) {
      return visit_ternary(*i);
    }

    if (auto i = dynamic_cast<TypeDecl*>(&n)) {
      return visit_typedecl(*i);
    }

    if (auto i = dynamic_cast<TypeExprID*>(&n)) {
      return visit_typeexprid(*i);
    }

    if (auto i = dynamic_cast<Undefine*>(&n)) {
      return visit_undefine(*i);
    }

    if (auto i = dynamic_cast<VarDecl*>(&n)) {
      return visit_vardecl(*i);
    }

    if (auto i = dynamic_cast<While*>(&n)) {
      return visit_while(*i);
    }

#ifndef NDEBUG
    std::cerr << "missed case in BaseTraversal::dispatch: " << typeid(n).name() << "\n";
#endif
    assert(!"missed case in BaseTraversal::dispatch");
  }

  virtual ~BaseTraversal() = default;
};

class Traversal : public BaseTraversal<> {

 public:
  void visit_add(Add &n) override;
  void visit_aliasdecl(AliasDecl &n) override;
  void visit_aliasrule(AliasRule &n) override;
  void visit_aliasstmt(AliasStmt &n) override;
  void visit_and(And &n) override;
  void visit_array(Array &n) override;
  void visit_assignment(Assignment &n) override;
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

  // Force class to be abstract
  virtual ~Traversal() = 0;

 private:
  void visit_bexpr(BinaryExpr &n);
  void visit_uexpr(UnaryExpr &n);
};

// Read-only equivalent of BaseTraversal.
template<typename RETURN_TYPE = void>
class ConstBaseTraversal {

 public:
  virtual RETURN_TYPE visit_add(const Add &n) = 0;
  virtual RETURN_TYPE visit_aliasdecl(const AliasDecl &n) = 0;
  virtual RETURN_TYPE visit_aliasrule(const AliasRule &n) = 0;
  virtual RETURN_TYPE visit_aliasstmt(const AliasStmt &n) = 0;
  virtual RETURN_TYPE visit_and(const And &n) = 0;
  virtual RETURN_TYPE visit_array(const Array &n) = 0;
  virtual RETURN_TYPE visit_assignment(const Assignment &n) = 0;
  virtual RETURN_TYPE visit_clear(const Clear &n) = 0;
  virtual RETURN_TYPE visit_constdecl(const ConstDecl &n) = 0;
  virtual RETURN_TYPE visit_div(const Div &n) = 0;
  virtual RETURN_TYPE visit_element(const Element &n) = 0;
  virtual RETURN_TYPE visit_enum(const Enum &n) = 0;
  virtual RETURN_TYPE visit_eq(const Eq &n) = 0;
  virtual RETURN_TYPE visit_errorstmt(const ErrorStmt &n) = 0;
  virtual RETURN_TYPE visit_exists(const Exists &n) = 0;
  virtual RETURN_TYPE visit_exprid(const ExprID &n) = 0;
  virtual RETURN_TYPE visit_field(const Field &n) = 0;
  virtual RETURN_TYPE visit_for(const For &n) = 0;
  virtual RETURN_TYPE visit_forall(const Forall &n) = 0;
  virtual RETURN_TYPE visit_function(const Function &n) = 0;
  virtual RETURN_TYPE visit_functioncall(const FunctionCall &n) = 0;
  virtual RETURN_TYPE visit_geq(const Geq &n) = 0;
  virtual RETURN_TYPE visit_gt(const Gt &n) = 0;
  virtual RETURN_TYPE visit_if(const If &n) = 0;
  virtual RETURN_TYPE visit_ifclause(const IfClause &n) = 0;
  virtual RETURN_TYPE visit_implication(const Implication &n) = 0;
  virtual RETURN_TYPE visit_isundefined(const IsUndefined &n) = 0;
  virtual RETURN_TYPE visit_leq(const Leq &n) = 0;
  virtual RETURN_TYPE visit_lt(const Lt &n) = 0;
  virtual RETURN_TYPE visit_model(const Model &n) = 0;
  virtual RETURN_TYPE visit_mod(const Mod &n) = 0;
  virtual RETURN_TYPE visit_mul(const Mul &n) = 0;
  virtual RETURN_TYPE visit_negative(const Negative &n) = 0;
  virtual RETURN_TYPE visit_neq(const Neq &n) = 0;
  virtual RETURN_TYPE visit_not(const Not &n) = 0;
  virtual RETURN_TYPE visit_number(const Number &n) = 0;
  virtual RETURN_TYPE visit_or(const Or &n) = 0;
  virtual RETURN_TYPE visit_procedurecall(const ProcedureCall &n) = 0;
  virtual RETURN_TYPE visit_property(const Property &n) = 0;
  virtual RETURN_TYPE visit_propertyrule(const PropertyRule &n) = 0;
  virtual RETURN_TYPE visit_propertystmt(const PropertyStmt &n) = 0;
  virtual RETURN_TYPE visit_put(const Put &n) = 0;
  virtual RETURN_TYPE visit_quantifier(const Quantifier &n) = 0;
  virtual RETURN_TYPE visit_range(const Range &n) = 0;
  virtual RETURN_TYPE visit_record(const Record &n) = 0;
  virtual RETURN_TYPE visit_return(const Return &n) = 0;
  virtual RETURN_TYPE visit_ruleset(const Ruleset &n) = 0;
  virtual RETURN_TYPE visit_scalarset(const Scalarset &n) = 0;
  virtual RETURN_TYPE visit_simplerule(const SimpleRule &n) = 0;
  virtual RETURN_TYPE visit_startstate(const StartState &n) = 0;
  virtual RETURN_TYPE visit_sub(const Sub &n) = 0;
  virtual RETURN_TYPE visit_switch(const Switch &n) = 0;
  virtual RETURN_TYPE visit_switchcase(const SwitchCase &n) = 0;
  virtual RETURN_TYPE visit_ternary(const Ternary &n) = 0;
  virtual RETURN_TYPE visit_typedecl(const TypeDecl &n) = 0;
  virtual RETURN_TYPE visit_typeexprid(const TypeExprID &n) = 0;
  virtual RETURN_TYPE visit_undefine(const Undefine &n) = 0;
  virtual RETURN_TYPE visit_vardecl(const VarDecl &n) = 0;
  virtual RETURN_TYPE visit_while(const While &n) = 0;

  RETURN_TYPE dispatch(const Node &n) {

    if (auto i = dynamic_cast<const Add*>(&n)) {
      return visit_add(*i);
    }

    if (auto i = dynamic_cast<const AliasDecl*>(&n)) {
      return visit_aliasdecl(*i);
    }

    if (auto i = dynamic_cast<const AliasRule*>(&n)) {
      return visit_aliasrule(*i);
    }

    if (auto i = dynamic_cast<const AliasStmt*>(&n)) {
      return visit_aliasstmt(*i);
    }

    if (auto i = dynamic_cast<const And*>(&n)) {
      return visit_and(*i);
    }

    if (auto i = dynamic_cast<const Array*>(&n)) {
      return visit_array(*i);
    }

    if (auto i = dynamic_cast<const Assignment*>(&n)) {
      return visit_assignment(*i);
    }

    if (auto i = dynamic_cast<const Clear*>(&n)) {
      return visit_clear(*i);
    }

    if (auto i = dynamic_cast<const ConstDecl*>(&n)) {
      return visit_constdecl(*i);
    }

    if (auto i = dynamic_cast<const Div*>(&n)) {
      return visit_div(*i);
    }

    if (auto i = dynamic_cast<const Element*>(&n)) {
      return visit_element(*i);
    }

    if (auto i = dynamic_cast<const Enum*>(&n)) {
      return visit_enum(*i);
    }

    if (auto i = dynamic_cast<const Eq*>(&n)) {
      return visit_eq(*i);
    }

    if (auto i = dynamic_cast<const ErrorStmt*>(&n)) {
      return visit_errorstmt(*i);
    }

    if (auto i = dynamic_cast<const Exists*>(&n)) {
      return visit_exists(*i);
    }

    if (auto i = dynamic_cast<const ExprID*>(&n)) {
      return visit_exprid(*i);
    }

    if (auto i = dynamic_cast<const Field*>(&n)) {
      return visit_field(*i);
    }

    if (auto i = dynamic_cast<const For*>(&n)) {
      return visit_for(*i);
    }

    if (auto i = dynamic_cast<const Forall*>(&n)) {
      return visit_forall(*i);
    }

    if (auto i = dynamic_cast<const Function*>(&n)) {
      return visit_function(*i);
    }

    if (auto i = dynamic_cast<const FunctionCall*>(&n)) {
      return visit_functioncall(*i);
    }

    if (auto i = dynamic_cast<const Geq*>(&n)) {
      return visit_geq(*i);
    }

    if (auto i = dynamic_cast<const Gt*>(&n)) {
      return visit_gt(*i);
    }

    if (auto i = dynamic_cast<const If*>(&n)) {
      return visit_if(*i);
    }

    if (auto i = dynamic_cast<const IfClause*>(&n)) {
      return visit_ifclause(*i);
    }

    if (auto i = dynamic_cast<const Implication*>(&n)) {
      return visit_implication(*i);
    }

    if (auto i = dynamic_cast<const IsUndefined*>(&n)) {
      return visit_isundefined(*i);
    }

    if (auto i = dynamic_cast<const Leq*>(&n)) {
      return visit_leq(*i);
    }

    if (auto i = dynamic_cast<const Lt*>(&n)) {
      return visit_lt(*i);
    }

    if (auto i = dynamic_cast<const Model*>(&n)) {
      return visit_model(*i);
    }

    if (auto i = dynamic_cast<const Mod*>(&n)) {
      return visit_mod(*i);
    }

    if (auto i = dynamic_cast<const Mul*>(&n)) {
      return visit_mul(*i);
    }

    if (auto i = dynamic_cast<const Negative*>(&n)) {
      return visit_negative(*i);
    }

    if (auto i = dynamic_cast<const Neq*>(&n)) {
      return visit_neq(*i);
    }

    if (auto i = dynamic_cast<const Not*>(&n)) {
      return visit_not(*i);
    }

    if (auto i = dynamic_cast<const Number*>(&n)) {
      return visit_number(*i);
    }

    if (auto i = dynamic_cast<const Or*>(&n)) {
      return visit_or(*i);
    }

    if (auto i = dynamic_cast<const ProcedureCall*>(&n)) {
      return visit_procedurecall(*i);
    }

    if (auto i = dynamic_cast<const Property*>(&n)) {
      return visit_property(*i);
    }

    if (auto i = dynamic_cast<const PropertyRule*>(&n)) {
      return visit_propertyrule(*i);
    }

    if (auto i = dynamic_cast<const PropertyStmt*>(&n)) {
      return visit_propertystmt(*i);
    }

    if (auto i = dynamic_cast<const Put*>(&n)) {
      return visit_put(*i);
    }

    if (auto i = dynamic_cast<const Quantifier*>(&n)) {
      return visit_quantifier(*i);
    }

    if (auto i = dynamic_cast<const Range*>(&n)) {
      return visit_range(*i);
    }

    if (auto i = dynamic_cast<const Record*>(&n)) {
      return visit_record(*i);
    }

    if (auto i = dynamic_cast<const Return*>(&n)) {
      return visit_return(*i);
    }

    if (auto i = dynamic_cast<const Ruleset*>(&n)) {
      return visit_ruleset(*i);
    }

    if (auto i = dynamic_cast<const Scalarset*>(&n)) {
      return visit_scalarset(*i);
    }

    if (auto i = dynamic_cast<const SimpleRule*>(&n)) {
      return visit_simplerule(*i);
    }

    if (auto i = dynamic_cast<const StartState*>(&n)) {
      return visit_startstate(*i);
    }

    if (auto i = dynamic_cast<const Sub*>(&n)) {
      return visit_sub(*i);
    }

    if (auto i = dynamic_cast<const Switch*>(&n)) {
      return visit_switch(*i);
    }

    if (auto i = dynamic_cast<const SwitchCase*>(&n)) {
      return visit_switchcase(*i);
    }

    if (auto i = dynamic_cast<const Ternary*>(&n)) {
      return visit_ternary(*i);
    }

    if (auto i = dynamic_cast<const TypeDecl*>(&n)) {
      return visit_typedecl(*i);
    }

    if (auto i = dynamic_cast<const TypeExprID*>(&n)) {
      return visit_typeexprid(*i);
    }

    if (auto i = dynamic_cast<const Undefine*>(&n)) {
      return visit_undefine(*i);
    }

    if (auto i = dynamic_cast<const VarDecl*>(&n)) {
      return visit_vardecl(*i);
    }

    if (auto i = dynamic_cast<const While*>(&n)) {
      return visit_while(*i);
    }

#ifndef NDEBUG
    std::cerr << "missed case in ConstBaseTraversal::dispatch: " << typeid(n).name() << "\n";
#endif
    assert(!"missed case in ConstBaseTraversal::dispatch");
  }

  virtual ~ConstBaseTraversal() = default;
};


// Read-only equivalent of Traversal.
class ConstTraversal : public ConstBaseTraversal<> {

 public:
  void visit_add(const Add &n) override;
  void visit_aliasdecl(const AliasDecl &n) override;
  void visit_aliasrule(const AliasRule &n) override;
  void visit_aliasstmt(const AliasStmt &n) override;
  void visit_and(const And &n) override;
  void visit_array(const Array &n) override;
  void visit_assignment(const Assignment &n) override;
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
class ConstExprTraversal : public ConstBaseTraversal<> {

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
class ConstStmtTraversal : public ConstBaseTraversal<> {

 public:
  void visit_add(const Add &n) final;
  void visit_aliasdecl(const AliasDecl &n) final;
  void visit_aliasrule(const AliasRule &n) final;
  void visit_and(const And &n) final;
  void visit_array(const Array &n) final;
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

  virtual ~ConstStmtTraversal() = default;

 private:
  void visit_bexpr(const BinaryExpr &n);
  void visit_uexpr(const UnaryExpr &n);
};

// Generic base for read-only traversals that only need to act on TypeExprs
class ConstTypeTraversal : public ConstBaseTraversal<> {

 public:
  void visit_add(const Add &n) final;
  void visit_aliasdecl(const AliasDecl &n) final;
  void visit_aliasrule(const AliasRule &n) final;
  void visit_aliasstmt(const AliasStmt &n) final;
  void visit_and(const And &n) final;
  void visit_assignment(const Assignment &n) final;
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

  virtual ~ConstTypeTraversal() = default;

 private:
  void visit_bexpr(const BinaryExpr &n);
  void visit_uexpr(const UnaryExpr &n);
};
}
