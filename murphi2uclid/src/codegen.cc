#include "codegen.h"
#include <cstddef>
#include <cassert>
#include <iostream>
#include <rumur/rumur.h>
#include <string>

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

  size_t indentation = 1; ///< current indentation level

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
    // assume we are within a procedure or init and so can use synchronous
    // assignment
    *this << tab() << *n.lhs << " = " << *n.rhs << ";\n";
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

    // emit as a symbolic constant
    *this << tab() << "const " << n.name << " : " << *n.get_type() << ";\n";

    // constrain it to have exactly its known value
    *this << tab() << "assume " << n.name << "_value: " << n.name << " == "
      << *n.value << ";\n";
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
    assert(!n.clauses.empty() && "if statement with no content");
    bool first = true;
    for (const IfClause &c : n.clauses) {
      if (first) {
        *this << tab() << c;
        first = false;
      } else {
        *this << " else {\n";
        indent();
        *this << tab() << c;
      }
    }
    *this << "\n";
  }

  void visit_ifclause(const IfClause &n) final {
    if (n.condition != nullptr) {
      *this << "if (" << *n.condition << ") {\n";
      indent();
    }
    for (const Ptr<Stmt> &s : n.body)
      *this << *s;
    dedent();
    *this << tab() << "}";
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
    for (const Ptr<Node> &c : n.children)
      *this << *c;
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
    *this << n.value.get_str();
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

  void visit_range(const Range&) final {
    *this << "integer";

    // TODO: range limits
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
    // TODO: how to deal with models that have more than one startstate?
    *this << "\n" << tab() << "init {\n";
    indent();

    for (const Ptr<Decl> &d : n.decls)
      *this << *d;

    for (const Ptr<Stmt> &s : n.body)
      *this << *s;

    dedent();
    *this << tab() << "}\n";
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
    // no need for special “boolean” handling because it has the same
    // spelling in Murphi and Uclid5
    *this << n.name;
  }

  void visit_undefine(const Undefine &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_vardecl(const VarDecl &n) final {
    // TODO: this will not work for Record fields
    *this << tab() << "var " << n.name << " : " << *n.get_type() << ";\n";
  }

  void visit_while(const While &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_xor(const Xor &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

private:
  // wrappers to allow more readable code above
  Printer &operator<<(const std::string &s) {
    o << s;
    return *this;
  }
  Printer &operator<<(const Node &n) {
    dispatch(n);
    return *this;
  }

  void indent() { ++indentation; }

  void dedent() {
    assert(indentation > 0);
    --indentation;
  }

  std::string tab() { return std::string(indentation * 2, ' '); }
};

} // namespace

void codegen(const Node &n, std::ostream &out) {
  Printer p(out);
  p.dispatch(n);
}
