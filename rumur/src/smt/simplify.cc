#include "simplify.h"
#include "../log.h"
#include "../options.h"
#include "define-enum-members.h"
#include "define-records.h"
#include "except.h"
#include "logic.h"
#include "solver.h"
#include "translate.h"
#include "typeexpr-to-smt.h"
#include <cassert>
#include <cstddef>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

namespace smt {

/* Simplification logic. We only attempt to replace tautologies with true and
 * contradictions with false, rather than going further and removing unreachable
 * code. We assume the C compiler building the generated verifier is clever
 * enough to make these transformations itself, so we leave them for it.
 */
namespace {
class Simplifier : public BaseTraversal {

private:
  Solver *solver;

public:
  explicit Simplifier(Solver &solver_) : solver(&solver_) {}

  /* if you are editing the visitation logic, note that the calls to
   * open_scope/close_scope are intended to match the pattern in
   * ../../../librumur/src/resolve-symbols.cc so that the SMT solver resolves
   * ExprIDs/TypeExprIDs the same way we do
   */

  void visit_add(Add &n) final { visit_bexpr(n); }

  void visit_aliasdecl(AliasDecl &n) final {
    dispatch(*n.value);
    simplify(n.value);
  }

  void visit_aliasrule(AliasRule &n) final {
    solver->open_scope();
    for (Ptr<AliasDecl> &alias : n.aliases) {
      dispatch(*alias);
      declare_decl(*alias);
    }
    for (Ptr<Rule> &rule : n.rules)
      dispatch(*rule);
    solver->close_scope();
  }

  void visit_aliasstmt(AliasStmt &n) final {
    solver->open_scope();
    for (Ptr<AliasDecl> &alias : n.aliases) {
      dispatch(*alias);
      declare_decl(*alias);
    }
    for (Ptr<Stmt> &stmt : n.body)
      dispatch(*stmt);
    solver->close_scope();
  }

  void visit_and(And &n) final { visit_bexpr(n); }

  void visit_array(Array &n) final {
    dispatch(*n.index_type);
    dispatch(*n.element_type);
  }

  void visit_assignment(Assignment &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);

    simplify(n.rhs);
  }

  void visit_band(Band &n) final { visit_bexpr(n); }
  void visit_bnot(Bnot &n) final { visit_uexpr(n); }
  void visit_bor(Bor &n) final { visit_bexpr(n); }

  void visit_clear(Clear &n) final {
    dispatch(*n.rhs);

    simplify(n.rhs);
  }

  void visit_constdecl(ConstDecl &n) final {
    dispatch(*n.value);
    simplify(n.value);

    if (n.type != nullptr)
      dispatch(*n.type);
  }

  void visit_div(Div &n) final { visit_bexpr(n); }

  void visit_element(Element &n) final {
    dispatch(*n.array);
    dispatch(*n.index);

    simplify(n.array);
    simplify(n.index);
  }

  void visit_enum(Enum &) final {
    // nothing required (see declare_decl)
  }

  void visit_eq(Eq &n) final { visit_bexpr(n); }
  void visit_errorstmt(ErrorStmt &) final {}

  void visit_exists(Exists &n) final {
    solver->open_scope();
    dispatch(n.quantifier);
    dispatch(*n.expr);

    simplify(n.expr);
    solver->close_scope();
  }

  void visit_exprid(ExprID &) final {}

  void visit_field(Field &n) final {
    dispatch(*n.record);

    simplify(n.record);
  }

  void visit_for(For &n) final {
    solver->open_scope();
    dispatch(n.quantifier);
    for (Ptr<Stmt> &stmt : n.body)
      dispatch(*stmt);
    solver->close_scope();
  }

  void visit_forall(Forall &n) final {
    solver->open_scope();
    dispatch(n.quantifier);
    dispatch(*n.expr);

    simplify(n.expr);
    solver->close_scope();
  }

  void visit_function(Function &n) final {
    solver->open_scope();
    for (Ptr<VarDecl> &parameter : n.parameters) {
      dispatch(*parameter);
      declare_decl(*parameter);
    }
    if (n.return_type != nullptr)
      dispatch(*n.return_type);
    for (Ptr<Decl> &decl : n.decls) {
      dispatch(*decl);
      declare_decl(*decl);
    }
    for (Ptr<Stmt> &stmt : n.body)
      dispatch(*stmt);
    solver->close_scope();
  }

  void visit_functioncall(FunctionCall &n) final {
    for (Ptr<Expr> &arg : n.arguments) {
      dispatch(*arg);
      simplify(arg);
    }
  }

  void visit_geq(Geq &n) final { visit_bexpr(n); }
  void visit_gt(Gt &n) final { visit_bexpr(n); }

  void visit_if(If &n) final {
    for (IfClause &c : n.clauses)
      dispatch(c);
  }

  void visit_ifclause(IfClause &n) final {

    if (n.condition != nullptr)
      dispatch(*n.condition);

    for (Ptr<Stmt> &s : n.body)
      dispatch(*s);

    if (n.condition != nullptr)
      simplify(n.condition);
  }

  void visit_implication(Implication &n) final { visit_bexpr(n); }
  void visit_isundefined(IsUndefined &n) final { visit_uexpr(n); }
  void visit_leq(Leq &n) final { visit_bexpr(n); }
  void visit_lsh(Lsh &n) final { visit_bexpr(n); }
  void visit_lt(Lt &n) final { visit_bexpr(n); }
  void visit_mod(Mod &n) final { visit_bexpr(n); }

  void visit_model(Model &n) final {
    solver->open_scope();
    for (Ptr<Node> &c : n.children) {
      dispatch(*c);
      if (auto d = dynamic_cast<Decl *>(c.get()))
        declare_decl(*d);
      if (auto f = dynamic_cast<Function *>(c.get()))
        declare_func(*f);
    }
    solver->open_scope();
  }

  void visit_mul(Mul &n) final { visit_bexpr(n); }
  void visit_negative(Negative &n) final { visit_uexpr(n); }
  void visit_neq(Neq &n) final { visit_bexpr(n); }
  void visit_not(Not &n) final { visit_uexpr(n); }
  void visit_number(Number &) final {}
  void visit_or(Or &n) final { visit_bexpr(n); }

  void visit_procedurecall(ProcedureCall &n) final { dispatch(n.call); }

  void visit_property(Property &) final {
    /* properties are printed to the user during verification, so don't simplify
     * the expression as it may confuse the user that it's not the same as what
     * they entered
     */
  }

  void visit_propertyrule(PropertyRule &n) final {
    for (Quantifier &quantifier : n.quantifiers)
      dispatch(quantifier);
    for (Ptr<AliasDecl> &alias : n.aliases)
      dispatch(*alias);
    dispatch(n.property);
  }

  void visit_propertystmt(PropertyStmt &n) final { dispatch(n.property); }

  void visit_put(Put &) final {
    /* deliberately do nothing here because the expression in a 'put' statement
     * is displayed to the user during verification and will surprise them if it
     * has been simplified into something other than what they wrote
     */
  }

  void visit_quantifier(Quantifier &n) final {
    if (n.type != nullptr) {
      dispatch(*n.type);

      declare_decl(*n.decl);
    } else {
      assert(n.from != nullptr);
      assert(n.to != nullptr);

      dispatch(*n.from);
      dispatch(*n.to);
      if (n.step != nullptr)
        dispatch(*n.step);

      simplify(n.from);
      simplify(n.to);
      if (n.step != nullptr)
        simplify(n.step);

      declare_decl(*n.decl);

      const std::string name = mangle(n.decl->name, n.decl->unique_id);
      if (n.from->constant()) {
        const std::string lb = numeric_literal(n.from->constant_fold());
        *solver << "(assert (" << geq() << " " << name << " " << lb << "))\n";
      }
      if (n.to->constant()) {
        const std::string ub = numeric_literal(n.to->constant_fold());
        *solver << "(assert (" << leq() << " " << name << " " << ub << "))\n";
      }
    }
  }

  void visit_range(Range &n) final {
    dispatch(*n.min);
    dispatch(*n.max);

    simplify(n.min);
    simplify(n.max);
  }

  void visit_record(Record &n) final {
    for (Ptr<VarDecl> &field : n.fields)
      dispatch(*field);
  }

  void visit_return(Return &n) final {
    if (n.expr != nullptr) {
      dispatch(*n.expr);

      simplify(n.expr);
    }
  }

  void visit_rsh(Rsh &n) final { visit_bexpr(n); }

  void visit_ruleset(Ruleset &n) final {
    solver->open_scope();
    for (Quantifier &quantifier : n.quantifiers)
      dispatch(quantifier);
    for (Ptr<AliasDecl> &alias : n.aliases)
      dispatch(*alias);
    for (Ptr<Rule> &rule : n.rules)
      dispatch(*rule);
    solver->close_scope();
  }

  void visit_scalarset(Scalarset &n) final {
    dispatch(*n.bound);
    simplify(n.bound);
  }

  void visit_simplerule(SimpleRule &n) final {
    solver->open_scope();
    for (Quantifier &quantifier : n.quantifiers)
      dispatch(quantifier);
    for (Ptr<AliasDecl> &alias : n.aliases)
      dispatch(*alias);
    if (n.guard != nullptr) {
      dispatch(*n.guard);
      simplify(n.guard);
    }
    for (Ptr<Decl> &decl : n.decls) {
      dispatch(*decl);
      declare_decl(*decl);
    }
    for (Ptr<Stmt> &stmt : n.body)
      dispatch(*stmt);
    solver->close_scope();
  }

  void visit_startstate(StartState &n) final {
    solver->open_scope();
    for (Quantifier &quantifier : n.quantifiers)
      dispatch(quantifier);
    for (Ptr<AliasDecl> &alias : n.aliases)
      dispatch(*alias);
    for (Ptr<Decl> &decl : n.decls) {
      dispatch(*decl);
      declare_decl(*decl);
    }
    for (Ptr<Stmt> &stmt : n.body)
      dispatch(*stmt);
    solver->close_scope();
  }

  void visit_sub(Sub &n) final { visit_bexpr(n); }

  void visit_switchcase(SwitchCase &n) final {
    for (Ptr<Expr> &match : n.matches) {
      dispatch(*match);
      simplify(match);
    }
    for (Ptr<Stmt> &stmt : n.body) {
      dispatch(*stmt);
    }
  }

  void visit_switch(Switch &n) final {
    dispatch(*n.expr);
    simplify(n.expr);

    for (SwitchCase &c : n.cases)
      dispatch(c);
  }

  void visit_ternary(Ternary &n) final {
    dispatch(*n.cond);
    dispatch(*n.lhs);
    dispatch(*n.rhs);

    simplify(n.cond);
    simplify(n.lhs);
    simplify(n.rhs);
  }

  void visit_typedecl(TypeDecl &n) final { dispatch(*n.value); }

  void visit_typeexprid(TypeExprID &) final {}

  void visit_undefine(Undefine &n) final { dispatch(*n.rhs); }

  void visit_vardecl(VarDecl &n) final { dispatch(*n.type); }

  void visit_while(While &n) final {
    dispatch(*n.condition);
    simplify(n.condition);

    for (Ptr<Stmt> &stmt : n.body)
      dispatch(*stmt);
  }

  void visit_xor(Xor &n) final { visit_bexpr(n); }

private:
  void visit_bexpr(BinaryExpr &n) {
    dispatch(*n.lhs);
    dispatch(*n.rhs);

    simplify(n.lhs);
    simplify(n.rhs);
  }

  void visit_uexpr(UnaryExpr &n) {
    dispatch(*n.rhs);

    simplify(n.rhs);
  }

  // try to squash a boolean expression to "True" or "False"
  void simplify(Ptr<Expr> &e) {

    assert(e != nullptr && "attempt to simplify a NULL expression");

    // we currently only handle boolean expressions
    if (!e->is_boolean())
      return;

    // there's no point trying to simplify a literal we will replace with itself
    if (e->is_literal_true() || e->is_literal_false())
      return;

    std::string claim;
    try {
      claim = translate(*e);
    } catch (Unsupported &) {
      *info << "skipping SMT simplification of unsupported expression \""
            << e->to_string() << "\"\n";
      return;
    }

    if (solver->is_true(claim)) {
      *info << "simplifying \"" << e->to_string() << "\" to true\n";
      e = make_true();
    } else if (solver->is_false(claim)) {
      *info << "simplifying \"" << e->to_string() << "\" to false\n";
      e = make_false();
    }
  }

  // invent a reference to "true"
  static Ptr<Expr> make_true() { return Ptr<Expr>(True); }

  // invent a reference to "false"
  static Ptr<Expr> make_false() { return Ptr<Expr>(False); }

  // declare a variable/type to the solver
  void declare_decl(const Decl &decl) {

    if (auto v = dynamic_cast<const VarDecl *>(&decl)) {
      declare_var(v->name, v->unique_id, *v->type);
      return;
    }

    if (auto c = dynamic_cast<const ConstDecl *>(&decl)) {

      if (c->type == nullptr) {
        // integer constant
        assert(c->value->constant() &&
               "non-constant value declared as constant");

        const std::string value = numeric_literal(c->value->constant_fold());

        const std::string name = mangle(c->name, c->unique_id);

        *solver << "(declare-fun " << name << " () " << integer_type() << ")\n"
                << "(assert (= " << name << " " << value << "))\n";

        return;
      }

      // TODO: enum constants
    }

    if (auto t = dynamic_cast<const TypeDecl *>(&decl)) {

      // define any enum members that occur as part of this TypeDecl
      define_enum_members(*solver, *t->value);

      // define any records that occur as part of this TypeDecl
      define_records(*solver, *t->value);

      const std::string my_name = mangle(t->name, t->unique_id);

      // nested TypeDecl (i.e. a typedecl of a typedecl)
      if (auto ref = dynamic_cast<const TypeExprID *>(t->value.get())) {
        const std::string ref_name =
            mangle(ref->name, ref->referent->unique_id);
        *solver << "(define-sort " << my_name << " () " << ref_name << ")\n";

      } else {
        // generic type definition
        *solver << "(define-sort " << my_name << " () "
                << typeexpr_to_smt(*t->value) << ")\n";
      }

      return;
    }

    // TODO
    throw Unsupported();
  }

  void declare_var(const std::string &name, size_t id, const TypeExpr &type) {

    // define any enum members that occur as part of the variable's type
    define_enum_members(*solver, type);

    // define any records that occur as part of this variable's type
    define_records(*solver, type);

    const std::string mangled = mangle(name, id);

    if (auto t = dynamic_cast<const TypeExprID *>(&type)) {
      // this has a previously defined type, so we know how to declare it
      const std::string tname = mangle(t->name, t->referent->unique_id);
      *solver << "(declare-fun " << mangled << " () " << tname << ")\n";
    } else {
      // otherwise declare it generically
      const std::string tname = typeexpr_to_smt(type);
      *solver << "(declare-fun " << mangled << " () " << tname << ")\n";
    }

    const Ptr<TypeExpr> t = type.resolve();

    // the solver already knows boolean, so we're done
    if (t->is_boolean())
      return;

    class ConstraintEmitter : public ConstTypeTraversal {

    private:
      Solver *solver;
      const std::string name;

    public:
      ConstraintEmitter(Solver &solver_, const std::string &name_)
          : solver(&solver_), name(name_) {}

      void visit_array(const Array &) final {
        // no constraints required
      }

      void visit_enum(const Enum &n) final {

        // constrain its values based on the number of enum members
        const std::string zero = numeric_literal(0);
        *solver << "(assert (" << geq() << " " << name << " " << zero << "))\n";
        const std::string size = numeric_literal(n.members.size());
        *solver << "(assert (" << lt() << " " << name << " " << size << "))\n";
      }

      void visit_range(const Range &n) final {

        // if this range's bounds are static, make them known to the solver
        if (n.constant()) {
          const std::string lb = numeric_literal(n.min->constant_fold());
          const std::string ub = numeric_literal(n.max->constant_fold());
          *solver << "(assert (" << geq() << " " << name << " " << lb << "))\n"
                  << "(assert (" << leq() << " " << name << " " << ub << "))\n";
        }
      }

      void visit_record(const Record &) final {
        // no constraints required
      }

      void visit_scalarset(const Scalarset &n) final {

        // scalarset values are at least 0
        const std::string zero = numeric_literal(0);
        *solver << "(assert (" << geq() << " " << name << " " << zero << "))\n";

        // if this scalarset's bounds are static, make them known to the solver
        if (n.constant()) {
          const std::string b = numeric_literal(n.bound->constant_fold());
          *solver << "(assert (" << lt() << " " << name << " " << b << "))\n";
        }
      }

      void visit_typeexprid(const TypeExprID &) final {
        assert(!"unreachable");
      }
    };

    ConstraintEmitter emitter(*solver, mangled);
    emitter.dispatch(*t);
  }

  void declare_func(const Function &) { throw Unsupported(); }
};
} // namespace

void simplify(Model &m) {

  // establish our connection to the solver
  Solver solver;

  // recursively traverse the model, simplifying as we go
  Simplifier simplifier(solver);
  simplifier.dispatch(m);
}

} // namespace smt
