#include <cstddef>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <rumur/traverse.h>
#include <rumur/validate.h>

using namespace rumur;

namespace {

class Validator : public ConstBaseTraversal {

public:
  void visit_add(const Add &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_aliasdecl(const AliasDecl &n) final {
    dispatch(*n.value);
    n.validate();
  }

  void visit_aliasrule(const AliasRule &n) final {
    for (auto &a : n.aliases)
      dispatch(*a);
    for (auto &r : n.rules)
      dispatch(*r);
    n.validate();
  }

  void visit_aliasstmt(const AliasStmt &n) final {
    for (auto &a : n.aliases)
      dispatch(*a);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_ambiguousamp(const AmbiguousAmp &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_ambiguouspipe(const AmbiguousPipe &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_and(const And &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_array(const Array &n) final {
    dispatch(*n.index_type);
    dispatch(*n.element_type);
    n.validate();
  }

  void visit_assignment(const Assignment &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_band(const Band &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_bnot(const Bnot &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_bor(const Bor &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_clear(const Clear &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_constdecl(const ConstDecl &n) final {
    dispatch(*n.value);
    n.validate();
  }

  void visit_div(const Div &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_element(const Element &n) final {
    dispatch(*n.array);
    dispatch(*n.index);
    n.validate();
  }

  void visit_enum(const Enum &n) final { n.validate(); }

  void visit_eq(const Eq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_errorstmt(const ErrorStmt &n) final { n.validate(); }

  void visit_exists(const Exists &n) final {
    dispatch(n.quantifier);
    dispatch(*n.expr);
    n.validate();
  }

  void visit_exprid(const ExprID &n) final {
    if (n.value != nullptr)
      dispatch(*n.value);
    n.validate();
  }

  void visit_field(const Field &n) final {
    dispatch(*n.record);
    n.validate();
  }

  void visit_for(const For &n) final {
    dispatch(n.quantifier);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_forall(const Forall &n) final {
    dispatch(n.quantifier);
    dispatch(*n.expr);
    n.validate();
  }

  void visit_function(const Function &n) final {
    for (auto &p : n.parameters)
      dispatch(*p);
    if (n.return_type != nullptr)
      dispatch(*n.return_type);
    for (auto &d : n.decls)
      dispatch(*d);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_functioncall(const FunctionCall &n) final {
    for (auto &a : n.arguments)
      dispatch(*a);
    n.validate();
  }

  void visit_geq(const Geq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_gt(const Gt &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_if(const If &n) final {
    for (const IfClause &c : n.clauses)
      dispatch(c);
    n.validate();
  }

  void visit_ifclause(const IfClause &n) final {
    if (n.condition != nullptr)
      dispatch(*n.condition);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_implication(const Implication &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_isundefined(const IsUndefined &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_leq(const Leq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_lsh(const Lsh &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_lt(const Lt &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_mod(const Mod &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_model(const Model &n) final {
    for (const Ptr<Node> &c : n.children)
      dispatch(*c);
    n.validate();
  }

  void visit_mul(const Mul &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_negative(const Negative &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_neq(const Neq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_not(const Not &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_number(const Number &n) final { n.validate(); }

  void visit_or(const Or &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_procedurecall(const ProcedureCall &n) final {
    dispatch(n.call);
    n.validate();
  }

  void visit_property(const Property &n) final {
    dispatch(*n.expr);
    n.validate();
  }

  void visit_propertyrule(const PropertyRule &n) final {
    for (const Quantifier &q : n.quantifiers)
      dispatch(q);
    dispatch(n.property);
    n.validate();
  }

  void visit_propertystmt(const PropertyStmt &n) final {
    dispatch(n.property);
    n.validate();
  }

  void visit_put(const Put &n) final {
    if (n.expr != nullptr)
      dispatch(*n.expr);
    n.validate();
  }

  void visit_quantifier(const Quantifier &n) final {
    if (n.type != nullptr)
      dispatch(*n.type);
    if (n.from != nullptr)
      dispatch(*n.from);
    if (n.to != nullptr)
      dispatch(*n.to);
    if (n.step != nullptr)
      dispatch(*n.step);
    n.validate();
  }

  void visit_range(const Range &n) final {
    dispatch(*n.min);
    dispatch(*n.max);
    n.validate();
  }

  void visit_record(const Record &n) final {
    for (auto &f : n.fields)
      dispatch(*f);
    n.validate();
  }

  void visit_return(const Return &n) final {
    if (n.expr != nullptr)
      dispatch(*n.expr);
    n.validate();
  }

  void visit_rsh(const Rsh &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_ruleset(const Ruleset &n) final {
    for (const Quantifier &q : n.quantifiers)
      dispatch(q);
    for (auto &r : n.rules)
      dispatch(*r);
    n.validate();
  }

  void visit_scalarset(const Scalarset &n) final {
    dispatch(*n.bound);
    n.validate();
  }

  void visit_simplerule(const SimpleRule &n) final {
    for (const Quantifier &q : n.quantifiers)
      dispatch(q);
    if (n.guard != nullptr)
      dispatch(*n.guard);
    for (auto &d : n.decls)
      dispatch(*d);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_startstate(const StartState &n) final {
    for (const Quantifier &q : n.quantifiers)
      dispatch(q);
    for (auto &d : n.decls)
      dispatch(*d);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_sub(const Sub &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_switch(const Switch &n) final {
    dispatch(*n.expr);
    for (const SwitchCase &c : n.cases)
      dispatch(c);
    n.validate();
  }

  void visit_switchcase(const SwitchCase &n) final {
    for (auto &m : n.matches)
      dispatch(*m);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_ternary(const Ternary &n) final {
    dispatch(*n.cond);
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_typedecl(const TypeDecl &n) final {
    dispatch(*n.value);
    n.validate();
  }

  void visit_typeexprid(const TypeExprID &n) final {
    if (n.referent != nullptr)
      dispatch(*n.referent);
    n.validate();
  }

  void visit_undefine(const Undefine &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit_vardecl(const VarDecl &n) final {
    if (n.type != nullptr)
      dispatch(*n.type);
    n.validate();
  }

  void visit_while(const While &n) final {
    dispatch(*n.condition);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit_xor(const Xor &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  virtual ~Validator() = default;
};

} // namespace

void rumur::validate(const Node &n) {
  Validator v;
  v.dispatch(n);
}
