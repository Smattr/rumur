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
    visit_add(*i);
    return;
  }

  if (auto i = dynamic_cast<AliasDecl*>(&n)) {
    visit_aliasdecl(*i);
    return;
  }

  if (auto i = dynamic_cast<AliasRule*>(&n)) {
    visit_aliasrule(*i);
    return;
  }

  if (auto i = dynamic_cast<AliasStmt*>(&n)) {
    visit_aliasstmt(*i);
    return;
  }

  if (auto i = dynamic_cast<And*>(&n)) {
    visit_and(*i);
    return;
  }

  if (auto i = dynamic_cast<Array*>(&n)) {
    visit_array(*i);
    return;
  }

  if (auto i = dynamic_cast<Assignment*>(&n)) {
    visit_assignment(*i);
    return;
  }

  if (auto i = dynamic_cast<ConstDecl*>(&n)) {
    visit_constdecl(*i);
    return;
  }

  if (auto i = dynamic_cast<Clear*>(&n)) {
    visit_clear(*i);
    return;
  }

  if (auto i = dynamic_cast<Div*>(&n)) {
    visit_div(*i);
    return;
  }

  if (auto i = dynamic_cast<Element*>(&n)) {
    visit_element(*i);
    return;
  }

  if (auto i = dynamic_cast<Function*>(&n)) {
    visit_function(*i);
    return;
  }

  if (auto i = dynamic_cast<FunctionCall*>(&n)) {
    visit_functioncall(*i);
    return;
  }

  if (auto i = dynamic_cast<Enum*>(&n)) {
    visit_enum(*i);
    return;
  }

  if (auto i = dynamic_cast<Eq*>(&n)) {
    visit_eq(*i);
    return;
  }

  if (auto i = dynamic_cast<ErrorStmt*>(&n)) {
    visit_errorstmt(*i);
    return;
  }

  if (auto i = dynamic_cast<Exists*>(&n)) {
    visit_exists(*i);
    return;
  }

  if (auto i = dynamic_cast<ExprID*>(&n)) {
    visit_exprid(*i);
    return;
  }

  if (auto i = dynamic_cast<Field*>(&n)) {
    visit_field(*i);
    return;
  }

  if (auto i = dynamic_cast<For*>(&n)) {
    visit_for(*i);
    return;
  }

  if (auto i = dynamic_cast<Forall*>(&n)) {
    visit_forall(*i);
    return;
  }

  if (auto i = dynamic_cast<Geq*>(&n)) {
    visit_geq(*i);
    return;
  }

  if (auto i = dynamic_cast<Gt*>(&n)) {
    visit_gt(*i);
    return;
  }

  if (auto i = dynamic_cast<If*>(&n)) {
    visit_if(*i);
    return;
  }

  if (auto i = dynamic_cast<IfClause*>(&n)) {
    visit_ifclause(*i);
    return;
  }

  if (auto i = dynamic_cast<Implication*>(&n)) {
    visit_implication(*i);
    return;
  }

  if (auto i = dynamic_cast<IsUndefined*>(&n)) {
    visit_isundefined(*i);
    return;
  }

  if (auto i = dynamic_cast<Leq*>(&n)) {
    visit_leq(*i);
    return;
  }

  if (auto i = dynamic_cast<Lt*>(&n)) {
    visit_lt(*i);
    return;
  }

  if (auto i = dynamic_cast<Model*>(&n)) {
    visit_model(*i);
    return;
  }

  if (auto i = dynamic_cast<Mod*>(&n)) {
    visit_mod(*i);
    return;
  }

  if (auto i = dynamic_cast<Mul*>(&n)) {
    visit_mul(*i);
    return;
  }

  if (auto i = dynamic_cast<Negative*>(&n)) {
    visit_negative(*i);
    return;
  }

  if (auto i = dynamic_cast<Neq*>(&n)) {
    visit_neq(*i);
    return;
  }

  if (auto i = dynamic_cast<Not*>(&n)) {
    visit_not(*i);
    return;
  }

  if (auto i = dynamic_cast<Number*>(&n)) {
    visit_number(*i);
    return;
  }

  if (auto i = dynamic_cast<Or*>(&n)) {
    visit_or(*i);
    return;
  }

  if (auto i = dynamic_cast<ProcedureCall*>(&n)) {
    visit_procedurecall(*i);
    return;
  }

  if (auto i = dynamic_cast<Property*>(&n)) {
    visit_property(*i);
    return;
  }

  if (auto i = dynamic_cast<PropertyRule*>(&n)) {
    visit_propertyrule(*i);
    return;
  }

  if (auto i = dynamic_cast<PropertyStmt*>(&n)) {
    visit_propertystmt(*i);
    return;
  }

  if (auto i = dynamic_cast<Put*>(&n)) {
    visit_put(*i);
    return;
  }

  if (auto i = dynamic_cast<Quantifier*>(&n)) {
    visit_quantifier(*i);
    return;
  }

  if (auto i = dynamic_cast<Range*>(&n)) {
    visit_range(*i);
    return;
  }

  if (auto i = dynamic_cast<Record*>(&n)) {
    visit_record(*i);
    return;
  }

  if (auto i = dynamic_cast<Return*>(&n)) {
    visit_return(*i);
    return;
  }

  if (auto i = dynamic_cast<Ruleset*>(&n)) {
    visit_ruleset(*i);
    return;
  }

  if (auto i = dynamic_cast<Scalarset*>(&n)) {
    visit_scalarset(*i);
    return;
  }

  if (auto i = dynamic_cast<SimpleRule*>(&n)) {
    visit_simplerule(*i);
    return;
  }

  if (auto i = dynamic_cast<StartState*>(&n)) {
    visit_startstate(*i);
    return;
  }

  if (auto i = dynamic_cast<Sub*>(&n)) {
    visit_sub(*i);
    return;
  }

  if (auto i = dynamic_cast<Switch*>(&n)) {
    visit_switch(*i);
    return;
  }

  if (auto i = dynamic_cast<SwitchCase*>(&n)) {
    visit_switchcase(*i);
    return;
  }

  if (auto i = dynamic_cast<Ternary*>(&n)) {
    visit_ternary(*i);
    return;
  }

  if (auto i = dynamic_cast<TypeDecl*>(&n)) {
    visit_typedecl(*i);
    return;
  }

  if (auto i = dynamic_cast<TypeExprID*>(&n)) {
    visit_typeexprid(*i);
    return;
  }

  if (auto i = dynamic_cast<Undefine*>(&n)) {
    visit_undefine(*i);
    return;
  }

  if (auto i = dynamic_cast<VarDecl*>(&n)) {
    visit_vardecl(*i);
    return;
  }

  if (auto i = dynamic_cast<While*>(&n)) {
    visit_while(*i);
    return;
  }

#ifndef NDEBUG
  std::cerr << "missed case in BaseTraversal::dispatch: " << typeid(n).name() << "\n";
#endif
  assert(!"missed case in BaseTraversal::dispatch");
}

void Traversal::visit_add(Add &n) {
  visit_bexpr(n);
}

void Traversal::visit_aliasdecl(AliasDecl &n) {
  dispatch(*n.value);
}

void Traversal::visit_aliasrule(AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Traversal::visit_aliasstmt(AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void Traversal::visit_and(And &n) {
  visit_bexpr(n);
}

void Traversal::visit_array(Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void Traversal::visit_assignment(Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Traversal::visit_bexpr(BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Traversal::visit_clear(Clear &n) {
  dispatch(*n.rhs);
}

void Traversal::visit_constdecl(ConstDecl &n) {
  dispatch(*n.value);
}

void Traversal::visit_div(Div &n) {
  visit_bexpr(n);
}

void Traversal::visit_element(Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void Traversal::visit_enum(Enum&) { }

void Traversal::visit_eq(Eq &n) {
  visit_bexpr(n);
}

void Traversal::visit_errorstmt(ErrorStmt&) { }

void Traversal::visit_exists(Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void Traversal::visit_exprid(ExprID&) { }

void Traversal::visit_field(Field &n) {
  dispatch(*n.record);
}

void Traversal::visit_for(For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void Traversal::visit_forall(Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void Traversal::visit_function(Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void Traversal::visit_functioncall(FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

void Traversal::visit_geq(Geq &n) {
  visit_bexpr(n);
}

void Traversal::visit_gt(Gt &n) {
  visit_bexpr(n);
}

void Traversal::visit_if(If &n) {
  for (IfClause &c : n.clauses)
    dispatch(c);
}

void Traversal::visit_ifclause(IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void Traversal::visit_implication(Implication &n) {
  visit_bexpr(n);
}

void Traversal::visit_isundefined(IsUndefined &n) {
  visit_uexpr(n);
}

void Traversal::visit_leq(Leq &n) {
  visit_bexpr(n);
}

void Traversal::visit_lt(Lt &n) {
  visit_bexpr(n);
}

void Traversal::visit_mod(Mod &n) {
  visit_bexpr(n);
}

void Traversal::visit_model(Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Traversal::visit_mul(Mul &n) {
  visit_bexpr(n);
}

void Traversal::visit_negative(Negative &n) {
  visit_uexpr(n);
}

void Traversal::visit_neq(Neq &n) {
  visit_bexpr(n);
}

void Traversal::visit_not(Not &n) {
  visit_uexpr(n);
}

void Traversal::visit_number(Number&) { }

void Traversal::visit_or(Or &n) {
  visit_bexpr(n);
}

void Traversal::visit_procedurecall(ProcedureCall &n) {
  dispatch(n.call);
}

void Traversal::visit_property(Property &n) {
  dispatch(*n.expr);
}

void Traversal::visit_propertyrule(PropertyRule &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void Traversal::visit_propertystmt(PropertyStmt &n) {
  dispatch(n.property);
}

void Traversal::visit_put(Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Traversal::visit_quantifier(Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void Traversal::visit_range(Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void Traversal::visit_record(Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

void Traversal::visit_return(Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Traversal::visit_ruleset(Ruleset &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Traversal::visit_scalarset(Scalarset &n) {
  dispatch(*n.bound);
}

void Traversal::visit_simplerule(SimpleRule &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void Traversal::visit_startstate(StartState &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void Traversal::visit_sub(Sub &n) {
  visit_bexpr(n);
}

void Traversal::visit_switch(Switch &n) {
  dispatch(*n.expr);
  for (SwitchCase &c : n.cases)
    dispatch(c);
}

void Traversal::visit_switchcase(SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void Traversal::visit_ternary(Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Traversal::visit_typedecl(TypeDecl &n) {
  dispatch(*n.value);
}

void Traversal::visit_typeexprid(TypeExprID&) { }

void Traversal::visit_uexpr(UnaryExpr &n) {
  dispatch(*n.rhs);
}

void Traversal::visit_undefine(Undefine &n) {
  dispatch(*n.rhs);
}

void Traversal::visit_vardecl(VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void Traversal::visit_while(While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

Traversal::~Traversal() { }

void ConstBaseTraversal::dispatch(const Node &n) {

  if (auto i = dynamic_cast<const Add*>(&n)) {
    visit_add(*i);
    return;
  }

  if (auto i = dynamic_cast<const AliasDecl*>(&n)) {
    visit_aliasdecl(*i);
    return;
  }

  if (auto i = dynamic_cast<const AliasRule*>(&n)) {
    visit_aliasrule(*i);
    return;
  }

  if (auto i = dynamic_cast<const AliasStmt*>(&n)) {
    visit_aliasstmt(*i);
    return;
  }

  if (auto i = dynamic_cast<const And*>(&n)) {
    visit_and(*i);
    return;
  }

  if (auto i = dynamic_cast<const Array*>(&n)) {
    visit_array(*i);
    return;
  }

  if (auto i = dynamic_cast<const Assignment*>(&n)) {
    visit_assignment(*i);
    return;
  }

  if (auto i = dynamic_cast<const Clear*>(&n)) {
    visit_clear(*i);
    return;
  }

  if (auto i = dynamic_cast<const ConstDecl*>(&n)) {
    visit_constdecl(*i);
    return;
  }

  if (auto i = dynamic_cast<const Div*>(&n)) {
    visit_div(*i);
    return;
  }

  if (auto i = dynamic_cast<const Element*>(&n)) {
    visit_element(*i);
    return;
  }

  if (auto i = dynamic_cast<const Enum*>(&n)) {
    visit_enum(*i);
    return;
  }

  if (auto i = dynamic_cast<const Eq*>(&n)) {
    visit_eq(*i);
    return;
  }

  if (auto i = dynamic_cast<const ErrorStmt*>(&n)) {
    visit_errorstmt(*i);
    return;
  }

  if (auto i = dynamic_cast<const Exists*>(&n)) {
    visit_exists(*i);
    return;
  }

  if (auto i = dynamic_cast<const ExprID*>(&n)) {
    visit_exprid(*i);
    return;
  }

  if (auto i = dynamic_cast<const Field*>(&n)) {
    visit_field(*i);
    return;
  }

  if (auto i = dynamic_cast<const For*>(&n)) {
    visit_for(*i);
    return;
  }

  if (auto i = dynamic_cast<const Forall*>(&n)) {
    visit_forall(*i);
    return;
  }

  if (auto i = dynamic_cast<const Function*>(&n)) {
    visit_function(*i);
    return;
  }

  if (auto i = dynamic_cast<const FunctionCall*>(&n)) {
    visit_functioncall(*i);
    return;
  }

  if (auto i = dynamic_cast<const Geq*>(&n)) {
    visit_geq(*i);
    return;
  }

  if (auto i = dynamic_cast<const Gt*>(&n)) {
    visit_gt(*i);
    return;
  }

  if (auto i = dynamic_cast<const If*>(&n)) {
    visit_if(*i);
    return;
  }

  if (auto i = dynamic_cast<const IfClause*>(&n)) {
    visit_ifclause(*i);
    return;
  }

  if (auto i = dynamic_cast<const Implication*>(&n)) {
    visit_implication(*i);
    return;
  }

  if (auto i = dynamic_cast<const IsUndefined*>(&n)) {
    visit_isundefined(*i);
    return;
  }

  if (auto i = dynamic_cast<const Leq*>(&n)) {
    visit_leq(*i);
    return;
  }

  if (auto i = dynamic_cast<const Lt*>(&n)) {
    visit_lt(*i);
    return;
  }

  if (auto i = dynamic_cast<const Model*>(&n)) {
    visit_model(*i);
    return;
  }

  if (auto i = dynamic_cast<const Mod*>(&n)) {
    visit_mod(*i);
    return;
  }

  if (auto i = dynamic_cast<const Mul*>(&n)) {
    visit_mul(*i);
    return;
  }

  if (auto i = dynamic_cast<const Negative*>(&n)) {
    visit_negative(*i);
    return;
  }

  if (auto i = dynamic_cast<const Neq*>(&n)) {
    visit_neq(*i);
    return;
  }

  if (auto i = dynamic_cast<const Not*>(&n)) {
    visit_not(*i);
    return;
  }

  if (auto i = dynamic_cast<const Number*>(&n)) {
    visit_number(*i);
    return;
  }

  if (auto i = dynamic_cast<const Or*>(&n)) {
    visit_or(*i);
    return;
  }

  if (auto i = dynamic_cast<const ProcedureCall*>(&n)) {
    visit_procedurecall(*i);
    return;
  }

  if (auto i = dynamic_cast<const Property*>(&n)) {
    visit_property(*i);
    return;
  }

  if (auto i = dynamic_cast<const PropertyRule*>(&n)) {
    visit_propertyrule(*i);
    return;
  }

  if (auto i = dynamic_cast<const PropertyStmt*>(&n)) {
    visit_propertystmt(*i);
    return;
  }

  if (auto i = dynamic_cast<const Put*>(&n)) {
    visit_put(*i);
    return;
  }

  if (auto i = dynamic_cast<const Quantifier*>(&n)) {
    visit_quantifier(*i);
    return;
  }

  if (auto i = dynamic_cast<const Range*>(&n)) {
    visit_range(*i);
    return;
  }

  if (auto i = dynamic_cast<const Record*>(&n)) {
    visit_record(*i);
    return;
  }

  if (auto i = dynamic_cast<const Return*>(&n)) {
    visit_return(*i);
    return;
  }

  if (auto i = dynamic_cast<const Ruleset*>(&n)) {
    visit_ruleset(*i);
    return;
  }

  if (auto i = dynamic_cast<const Scalarset*>(&n)) {
    visit_scalarset(*i);
    return;
  }

  if (auto i = dynamic_cast<const SimpleRule*>(&n)) {
    visit_simplerule(*i);
    return;
  }

  if (auto i = dynamic_cast<const StartState*>(&n)) {
    visit_startstate(*i);
    return;
  }

  if (auto i = dynamic_cast<const Sub*>(&n)) {
    visit_sub(*i);
    return;
  }

  if (auto i = dynamic_cast<const Switch*>(&n)) {
    visit_switch(*i);
    return;
  }

  if (auto i = dynamic_cast<const SwitchCase*>(&n)) {
    visit_switchcase(*i);
    return;
  }

  if (auto i = dynamic_cast<const Ternary*>(&n)) {
    visit_ternary(*i);
    return;
  }

  if (auto i = dynamic_cast<const TypeDecl*>(&n)) {
    visit_typedecl(*i);
    return;
  }

  if (auto i = dynamic_cast<const TypeExprID*>(&n)) {
    visit_typeexprid(*i);
    return;
  }

  if (auto i = dynamic_cast<const Undefine*>(&n)) {
    visit_undefine(*i);
    return;
  }

  if (auto i = dynamic_cast<const VarDecl*>(&n)) {
    visit_vardecl(*i);
    return;
  }

  if (auto i = dynamic_cast<const While*>(&n)) {
    visit_while(*i);
    return;
  }

#ifndef NDEBUG
  std::cerr << "missed case in ConstBaseTraversal::dispatch: " << typeid(n).name() << "\n";
#endif
  assert(!"missed case in ConstBaseTraversal::dispatch");
}

void ConstTraversal::visit_add(const Add &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_aliasdecl(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstTraversal::visit_aliasrule(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit_aliasstmt(const AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit_and(const And &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_array(const Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void ConstTraversal::visit_assignment(const Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit_bexpr(const BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit_clear(const Clear &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit_constdecl(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstTraversal::visit_div(const Div &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_element(const Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void ConstTraversal::visit_enum(const Enum&) { }

void ConstTraversal::visit_eq(const Eq &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_errorstmt(const ErrorStmt&) { }

void ConstTraversal::visit_exists(const Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTraversal::visit_exprid(const ExprID&) { }

void ConstTraversal::visit_field(const Field &n) {
  dispatch(*n.record);
}

void ConstTraversal::visit_for(const For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit_forall(const Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTraversal::visit_function(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit_functioncall(const FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

void ConstTraversal::visit_geq(const Geq &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_gt(const Gt &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_if(const If &n) {
  for (const IfClause &c : n.clauses)
    dispatch(c);
}

void ConstTraversal::visit_ifclause(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit_implication(const Implication &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_isundefined(const IsUndefined &n) {
  visit_uexpr(n);
}

void ConstTraversal::visit_leq(const Leq &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_lt(const Lt &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_mod(const Mod &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_model(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit_mul(const Mul &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_negative(const Negative &n) {
  visit_uexpr(n);
}

void ConstTraversal::visit_neq(const Neq &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_not(const Not &n) {
  visit_uexpr(n);
}

void ConstTraversal::visit_number(const Number&) { }

void ConstTraversal::visit_or(const Or &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_procedurecall(const ProcedureCall &n) {
  dispatch(n.call);
}

void ConstTraversal::visit_property(const Property &n) {
  dispatch(*n.expr);
}

void ConstTraversal::visit_propertyrule(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstTraversal::visit_propertystmt(const PropertyStmt &n) {
  dispatch(n.property);
}

void ConstTraversal::visit_put(const Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTraversal::visit_quantifier(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstTraversal::visit_range(const Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void ConstTraversal::visit_record(const Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

void ConstTraversal::visit_return(const Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTraversal::visit_ruleset(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTraversal::visit_scalarset(const Scalarset &n) {
  dispatch(*n.bound);
}

void ConstTraversal::visit_simplerule(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit_startstate(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit_sub(const Sub &n) {
  visit_bexpr(n);
}

void ConstTraversal::visit_switch(const Switch &n) {
  dispatch(*n.expr);
  for (const SwitchCase &c : n.cases)
    dispatch(c);
}

void ConstTraversal::visit_switchcase(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTraversal::visit_ternary(const Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTraversal::visit_typedecl(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstTraversal::visit_typeexprid(const TypeExprID&) { }

void ConstTraversal::visit_uexpr(const UnaryExpr &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit_undefine(const Undefine &n) {
  dispatch(*n.rhs);
}

void ConstTraversal::visit_vardecl(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstTraversal::visit_while(const While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

ConstTraversal::~ConstTraversal() { }

void ConstExprTraversal::visit_aliasdecl(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstExprTraversal::visit_aliasrule(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstExprTraversal::visit_aliasstmt(const AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit_array(const Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void ConstExprTraversal::visit_assignment(const Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstExprTraversal::visit_clear(const Clear &n) {
  dispatch(*n.rhs);
}

void ConstExprTraversal::visit_constdecl(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstExprTraversal::visit_enum(const Enum&) { }

void ConstExprTraversal::visit_errorstmt(const ErrorStmt&) { }

void ConstExprTraversal::visit_for(const For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit_function(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit_if(const If &n) {
  for (const IfClause &c : n.clauses)
    dispatch(c);
}

void ConstExprTraversal::visit_ifclause(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit_model(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstExprTraversal::visit_procedurecall(const ProcedureCall &n) {
  dispatch(n.call);
}

void ConstExprTraversal::visit_property(const Property &n) {
  dispatch(*n.expr);
}

void ConstExprTraversal::visit_propertyrule(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstExprTraversal::visit_propertystmt(const PropertyStmt &n) {
  dispatch(n.property);
}

void ConstExprTraversal::visit_put(const Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstExprTraversal::visit_quantifier(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstExprTraversal::visit_range(const Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void ConstExprTraversal::visit_record(const Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

void ConstExprTraversal::visit_return(const Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstExprTraversal::visit_ruleset(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstExprTraversal::visit_scalarset(const Scalarset &n) {
  dispatch(*n.bound);
}

void ConstExprTraversal::visit_simplerule(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit_startstate(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit_switch(const Switch &n) {
  dispatch(*n.expr);
  for (const SwitchCase &c : n.cases)
    dispatch(c);
}

void ConstExprTraversal::visit_switchcase(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstExprTraversal::visit_typedecl(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstExprTraversal::visit_typeexprid(const TypeExprID&) { }

void ConstExprTraversal::visit_undefine(const Undefine &n) {
  dispatch(*n.rhs);
}

void ConstExprTraversal::visit_vardecl(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstExprTraversal::visit_while(const While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit_aliasdecl(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstStmtTraversal::visit_aliasrule(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstStmtTraversal::visit_add(const Add &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_and(const And &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_array(const Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void ConstStmtTraversal::visit_bexpr(const BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstStmtTraversal::visit_constdecl(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstStmtTraversal::visit_div(const Div &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_element(const Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void ConstStmtTraversal::visit_enum(const Enum&) { }

void ConstStmtTraversal::visit_eq(const Eq &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_exists(const Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstStmtTraversal::visit_exprid(const ExprID&) { }

void ConstStmtTraversal::visit_field(const Field &n) {
  dispatch(*n.record);
}

void ConstStmtTraversal::visit_forall(const Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstStmtTraversal::visit_function(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit_functioncall(const FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

void ConstStmtTraversal::visit_geq(const Geq &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_gt(const Gt &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_ifclause(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit_implication(const Implication &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_isundefined(const IsUndefined &n) {
  visit_uexpr(n);
}

void ConstStmtTraversal::visit_leq(const Leq &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_lt(const Lt &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_mod(const Mod &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_model(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstStmtTraversal::visit_mul(const Mul &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_negative(const Negative &n) {
  visit_uexpr(n);
}

void ConstStmtTraversal::visit_neq(const Neq &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_not(const Not &n) {
  visit_uexpr(n);
}

void ConstStmtTraversal::visit_number(const Number&) { }

void ConstStmtTraversal::visit_or(const Or &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_property(const Property &n) {
  dispatch(*n.expr);
}

void ConstStmtTraversal::visit_propertyrule(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstStmtTraversal::visit_quantifier(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstStmtTraversal::visit_range(const Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

void ConstStmtTraversal::visit_record(const Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

void ConstStmtTraversal::visit_ruleset(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstStmtTraversal::visit_scalarset(const Scalarset &n) {
  dispatch(*n.bound);
}

void ConstStmtTraversal::visit_simplerule(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit_startstate(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit_switchcase(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstStmtTraversal::visit_sub(const Sub &n) {
  visit_bexpr(n);
}

void ConstStmtTraversal::visit_ternary(const Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstStmtTraversal::visit_typedecl(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstStmtTraversal::visit_typeexprid(const TypeExprID&) { }

void ConstStmtTraversal::visit_uexpr(const UnaryExpr &n) {
  dispatch(*n.rhs);
}

void ConstStmtTraversal::visit_vardecl(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstTypeTraversal::visit_add(const Add &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_aliasdecl(const AliasDecl &n) {
  dispatch(*n.value);
}

void ConstTypeTraversal::visit_aliasrule(const AliasRule &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTypeTraversal::visit_aliasstmt(const AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit_and(const And &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_assignment(const Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit_bexpr(const BinaryExpr &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit_clear(const Clear &n) {
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit_constdecl(const ConstDecl &n) {
  dispatch(*n.value);
}

void ConstTypeTraversal::visit_div(const Div &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_element(const Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

void ConstTypeTraversal::visit_eq(const Eq &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_errorstmt(const ErrorStmt&) { }

void ConstTypeTraversal::visit_exists(const Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTypeTraversal::visit_exprid(const ExprID&) { }

void ConstTypeTraversal::visit_field(const Field &n) {
  dispatch(*n.record);
}

void ConstTypeTraversal::visit_for(const For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit_forall(const Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void ConstTypeTraversal::visit_function(const Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit_functioncall(const FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

void ConstTypeTraversal::visit_geq(const Geq &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_gt(const Gt &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_if(const If &n) {
  for (const IfClause &c : n.clauses)
    dispatch(c);
}

void ConstTypeTraversal::visit_ifclause(const IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit_implication(const Implication &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_isundefined(const IsUndefined &n) {
  visit_uexpr(n);
}

void ConstTypeTraversal::visit_leq(const Leq &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_lt(const Lt &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_mod(const Mod &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_model(const Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTypeTraversal::visit_mul(const Mul &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_negative(const Negative &n) {
  visit_uexpr(n);
}

void ConstTypeTraversal::visit_neq(const Neq &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_not(const Not &n) {
  visit_uexpr(n);
}

void ConstTypeTraversal::visit_number(const Number&) { }

void ConstTypeTraversal::visit_or(const Or &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_procedurecall(const ProcedureCall &n) {
  dispatch(n.call);
}

void ConstTypeTraversal::visit_property(const Property &n) {
  dispatch(*n.expr);
}

void ConstTypeTraversal::visit_propertyrule(const PropertyRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void ConstTypeTraversal::visit_propertystmt(const PropertyStmt &n) {
  dispatch(n.property);
}

void ConstTypeTraversal::visit_put(const Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTypeTraversal::visit_quantifier(const Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

void ConstTypeTraversal::visit_return(const Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void ConstTypeTraversal::visit_ruleset(const Ruleset &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void ConstTypeTraversal::visit_simplerule(const SimpleRule &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit_startstate(const StartState &n) {
  for (const Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit_sub(const Sub &n) {
  visit_bexpr(n);
}

void ConstTypeTraversal::visit_switch(const Switch &n) {
  dispatch(*n.expr);
  for (const SwitchCase &c : n.cases)
    dispatch(c);
}

void ConstTypeTraversal::visit_switchcase(const SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void ConstTypeTraversal::visit_ternary(const Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit_typedecl(const TypeDecl &n) {
  dispatch(*n.value);
}

void ConstTypeTraversal::visit_uexpr(const UnaryExpr &n) {
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit_undefine(const Undefine &n) {
  dispatch(*n.rhs);
}

void ConstTypeTraversal::visit_vardecl(const VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

void ConstTypeTraversal::visit_while(const While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

}
