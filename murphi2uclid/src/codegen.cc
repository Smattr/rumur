#include "codegen.h"
#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

/** a visitor that prints Uclid5 code
 *
 * While murphi2uclid only attempts to translate Models to Uclid5, this visitor
 * is actually capable of starting translation at a Node of any type.
 */
class Printer : public ConstBaseTraversal {

private:
  std::ostream &o; ///< output stream to emit code to

public:
  Printer(std::ostream &o_) : o(o_) {}

  void visit_add(const Add &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_aliasdecl(const AliasDecl &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_aliasrule(const AliasRule &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_aliasstmt(const AliasStmt &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_and(const And &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_array(const Array &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_assignment(const Assignment &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_band(const Band &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_bnot(const Bnot &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_bor(const Bor &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_clear(const Clear &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_constdecl(const ConstDecl &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_div(const Div &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_element(const Element &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_enum(const Enum &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_eq(const Eq &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_errorstmt(const ErrorStmt &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_exists(const Exists &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_exprid(const ExprID &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_field(const Field &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_for(const For &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_forall(const Forall &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_function(const Function &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_functioncall(const FunctionCall &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_geq(const Geq &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_gt(const Gt &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_if(const If &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_ifclause(const IfClause &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_implication(const Implication &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_isundefined(const IsUndefined &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_leq(const Leq &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_lsh(const Lsh &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_lt(const Lt &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_mod(const Mod &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_model(const Model &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_mul(const Mul &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_negative(const Negative &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_neq(const Neq &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_not(const Not &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_number(const Number &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_or(const Or &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_procedurecall(const ProcedureCall &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_property(const Property &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_propertyrule(const PropertyRule &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_propertystmt(const PropertyStmt &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_put(const Put &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_quantifier(const Quantifier &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_range(const Range &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_record(const Record &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_return(const Return &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_rsh(const Rsh &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_ruleset(const Ruleset &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_scalarset(const Scalarset &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_simplerule(const SimpleRule &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_startstate(const StartState &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_sub(const Sub &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_switch(const Switch &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_switchcase(const SwitchCase &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_ternary(const Ternary &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_typedecl(const TypeDecl &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_typeexprid(const TypeExprID &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_undefine(const Undefine &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_vardecl(const VarDecl &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_while(const While &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_xor(const Xor &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }
};

} // namespace

void codegen(const Node &n, std::ostream &out) {
  Printer p(out);
  p.dispatch(n);
}
