#include <cassert>
#include <cstddef>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <rumur/indexer.h>
#include <rumur/traverse.h>

namespace rumur {

void Indexer::visit_add(Add &n) { visit_bexpr(n); }

void Indexer::visit_aliasdecl(AliasDecl &n) {
  n.unique_id = next++;
  dispatch(*n.value);
}

void Indexer::visit_aliasrule(AliasRule &n) {
  n.unique_id = next++;
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Indexer::visit_aliasstmt(AliasStmt &n) {
  n.unique_id = next++;
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit_ambiguousamp(AmbiguousAmp &n) { visit_bexpr(n); }

void Indexer::visit_ambiguouspipe(AmbiguousPipe &n) { visit_bexpr(n); }

void Indexer::visit_and(And &n) { visit_bexpr(n); }

void Indexer::visit_array(Array &n) {
  n.unique_id = next++;
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

void Indexer::visit_assignment(Assignment &n) {
  n.unique_id = next++;
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Indexer::visit_band(Band &n) { visit_bexpr(n); }

void Indexer::visit_bnot(Bnot &n) { visit_uexpr(n); }

void Indexer::visit_bor(Bor &n) { visit_bexpr(n); }

void Indexer::visit_bexpr(BinaryExpr &n) {
  n.unique_id = next++;
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Indexer::visit_clear(Clear &n) {
  n.unique_id = next++;
  dispatch(*n.rhs);
}

void Indexer::visit_constdecl(ConstDecl &n) {
  n.unique_id = next++;
  dispatch(*n.value);
}

void Indexer::visit_div(Div &n) { visit_bexpr(n); }

void Indexer::visit_element(Element &n) {
  n.unique_id = next++;
  dispatch(*n.array);
  dispatch(*n.index);
}

void Indexer::visit_enum(Enum &n) {
  n.unique_id = next++;
  // allocate a block of IDs that this Enum can use for its members during type
  // checking (see resolve_symbols())
  n.unique_id_limit = next + n.members.size();
  next = n.unique_id_limit;
}

void Indexer::visit_eq(Eq &n) { visit_bexpr(n); }

void Indexer::visit_errorstmt(ErrorStmt &n) { n.unique_id = next++; }

void Indexer::visit_exists(Exists &n) {
  n.unique_id = next++;
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void Indexer::visit_exprid(ExprID &n) { n.unique_id = next++; }

void Indexer::visit_field(Field &n) {
  n.unique_id = next++;
  dispatch(*n.record);
}

void Indexer::visit_for(For &n) {
  n.unique_id = next++;
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit_forall(Forall &n) {
  n.unique_id = next++;
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

void Indexer::visit_function(Function &n) {
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

void Indexer::visit_functioncall(FunctionCall &n) {
  n.unique_id = next++;
  for (auto &a : n.arguments)
    dispatch(*a);
}

void Indexer::visit_geq(Geq &n) { visit_bexpr(n); }

void Indexer::visit_gt(Gt &n) { visit_bexpr(n); }

void Indexer::visit_if(If &n) {
  n.unique_id = next++;
  for (IfClause &c : n.clauses)
    dispatch(c);
}

void Indexer::visit_ifclause(IfClause &n) {
  n.unique_id = next++;
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit_implication(Implication &n) { visit_bexpr(n); }

void Indexer::visit_isundefined(IsUndefined &n) { visit_uexpr(n); }

void Indexer::visit_leq(Leq &n) { visit_bexpr(n); }

void Indexer::visit_lsh(Lsh &n) { visit_bexpr(n); }

void Indexer::visit_lt(Lt &n) { visit_bexpr(n); }

void Indexer::visit_mod(Mod &n) { visit_bexpr(n); }

void Indexer::visit_model(Model &n) {
  n.unique_id = next++;
  for (Ptr<Node> &c : n.children)
    dispatch(*c);
}

void Indexer::visit_mul(Mul &n) { visit_bexpr(n); }

void Indexer::visit_negative(Negative &n) { visit_uexpr(n); }

void Indexer::visit_neq(Neq &n) { visit_bexpr(n); }

void Indexer::visit_not(Not &n) { visit_uexpr(n); }

void Indexer::visit_number(Number &n) { n.unique_id = next++; }

void Indexer::visit_or(Or &n) { visit_bexpr(n); }

void Indexer::visit_procedurecall(ProcedureCall &n) {
  n.unique_id = next++;
  dispatch(n.call);
}

void Indexer::visit_property(Property &n) {
  n.unique_id = next++;
  dispatch(*n.expr);
}

void Indexer::visit_propertyrule(PropertyRule &n) {
  n.unique_id = next++;
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

void Indexer::visit_propertystmt(PropertyStmt &n) {
  n.unique_id = next++;
  dispatch(n.property);
}

void Indexer::visit_put(Put &n) {
  n.unique_id = next++;
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Indexer::visit_quantifier(Quantifier &n) {
  n.unique_id = next++;
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
  dispatch(*n.decl);
}

void Indexer::visit_range(Range &n) {
  n.unique_id = next++;
  dispatch(*n.min);
  dispatch(*n.max);
}

void Indexer::visit_record(Record &n) {
  n.unique_id = next++;
  for (auto &f : n.fields)
    dispatch(*f);
}

void Indexer::visit_return(Return &n) {
  n.unique_id = next++;
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

void Indexer::visit_rsh(Rsh &n) { visit_bexpr(n); }

void Indexer::visit_ruleset(Ruleset &n) {
  n.unique_id = next++;
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

void Indexer::visit_scalarset(Scalarset &n) {
  n.unique_id = next++;
  dispatch(*n.bound);
}

void Indexer::visit_simplerule(SimpleRule &n) {
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

void Indexer::visit_startstate(StartState &n) {
  n.unique_id = next++;
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit_sub(Sub &n) { visit_bexpr(n); }

void Indexer::visit_switch(Switch &n) {
  n.unique_id = next++;
  dispatch(*n.expr);
  for (SwitchCase &c : n.cases)
    dispatch(c);
}

void Indexer::visit_switchcase(SwitchCase &n) {
  n.unique_id = next++;
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit_ternary(Ternary &n) {
  n.unique_id = next++;
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

void Indexer::visit_typedecl(TypeDecl &n) {
  n.unique_id = next++;
  dispatch(*n.value);
}

void Indexer::visit_typeexprid(TypeExprID &n) { n.unique_id = next++; }

void Indexer::visit_uexpr(UnaryExpr &n) {
  n.unique_id = next++;
  dispatch(*n.rhs);
}

void Indexer::visit_undefine(Undefine &n) {
  n.unique_id = next++;
  dispatch(*n.rhs);
}

void Indexer::visit_vardecl(VarDecl &n) {
  n.unique_id = next++;
  dispatch(*n.type);
}

void Indexer::visit_while(While &n) {
  n.unique_id = next++;
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

void Indexer::visit_xor(Xor &n) { visit_bexpr(n); }

} // namespace rumur
