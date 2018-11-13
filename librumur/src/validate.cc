#include <cstddef>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Property.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <rumur/validate.h>

namespace rumur {

namespace {

class Validator : public ConstBaseTraversal {

 public:
  void visit(const Add &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const AliasDecl &n) final {
    dispatch(*n.value);
    n.validate();
  }

  void visit(const AliasRule &n) final {
    for (const std::shared_ptr<AliasDecl> &a : n.aliases)
      dispatch(*a);
    for (const std::shared_ptr<Rule> &r : n.rules)
      dispatch(*r);
    n.validate();
  }

  void visit(const AliasStmt &n) final {
    for (const std::shared_ptr<AliasDecl> &a : n.aliases)
      dispatch(*a);
    for (auto &s : n.body)
      dispatch(*s);
  }

  void visit(const And &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Array &n) final {
    dispatch(*n.index_type);
    dispatch(*n.element_type);
    n.validate();
  }

  void visit(const Assignment &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Clear &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const ConstDecl &n) final {
    dispatch(*n.value);
    n.validate();
  }

  void visit(const Div &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Element &n) final {
    dispatch(*n.array);
    dispatch(*n.index);
    n.validate();
  }

  void visit(const Enum &n) final {
    n.validate();
  }

  void visit(const Eq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const ErrorStmt &n) final {
    n.validate();
  }

  void visit(const Exists &n) final {
    dispatch(*n.quantifier);
    dispatch(*n.expr);
    n.validate();
  }

  void visit(const ExprID &n) final {
    /* Don't descend into *n.value because we will already validate this
     * elsewhere.
     */
    n.validate();
  }

  void visit(const Field &n) final {
    dispatch(*n.record);
    n.validate();
  }

  void visit(const For &n) final {
    dispatch(*n.quantifier);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit(const Forall &n) final {
    dispatch(*n.quantifier);
    dispatch(*n.expr);
    n.validate();
  }

  void visit(const Function &n) final {
    for (const std::shared_ptr<VarDecl> &p : n.parameters)
      dispatch(*p);
    if (n.return_type != nullptr)
      dispatch(*n.return_type);
    for (const std::shared_ptr<Decl> &d : n.decls)
      dispatch(*d);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit(const FunctionCall &n) final {
    for (const std::shared_ptr<Expr> &a : n.arguments)
      dispatch(*a);
    n.validate();
  }

  void visit(const Geq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Gt &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const If &n) final {
    for (const IfClause &c : n.clauses)
      dispatch(c);
    n.validate();
  }

  void visit(const IfClause &n) final {
    if (n.condition != nullptr)
      dispatch(*n.condition);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit(const Implication &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Leq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Lt &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Mod &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Model &n) final {
    for (const std::shared_ptr<Decl> &d : n.decls)
      dispatch(*d);
    for (const std::shared_ptr<Function> &f : n.functions)
      dispatch(*f);
    for (const std::shared_ptr<Rule> &r : n.rules)
      dispatch(*r);
    n.validate();
  }

  void visit(const Mul &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Negative &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Neq &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Not &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Number &n) final {
    n.validate();
  }

  void visit(const Or &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const ProcedureCall &n) final {
    for (const std::shared_ptr<Expr> &a : n.arguments)
      dispatch(*a);
    n.validate();
  }

  void visit(const Property &n) final {
    dispatch(*n.expr);
    n.validate();
  }

  void visit(const PropertyRule &n) final {
    for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
      dispatch(*q);
    dispatch(n.property);
    n.validate();
  }

  void visit(const PropertyStmt &n) final {
    dispatch(n.property);
    n.validate();
  }

  void visit(const Quantifier &n) final {
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

  void visit(const Range &n) final {
    dispatch(*n.min);
    dispatch(*n.max);
    n.validate();
  }

  void visit(const Record &n) final {
    for (const std::shared_ptr<VarDecl> &f : n.fields)
      dispatch(*f);
    n.validate();
  }

  void visit(const Return &n) final {
    if (n.expr != nullptr)
      dispatch(*n.expr);
    n.validate();
  }

  void visit(const Ruleset &n) final {
    for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
      dispatch(*q);
    for (const std::shared_ptr<Rule> &r : n.rules)
      dispatch(*r);
    n.validate();
  }

  void visit(const Scalarset &n) final {
    dispatch(*n.bound);
    n.validate();
  }

  void visit(const SimpleRule &n) final {
    for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
      dispatch(*q);
    if (n.guard != nullptr)
      dispatch(*n.guard);
    for (const std::shared_ptr<Decl> &d : n.decls)
      dispatch(*d);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit(const StartState &n) final {
    for (const std::shared_ptr<Quantifier> &q : n.quantifiers)
      dispatch(*q);
    for (const std::shared_ptr<Decl> &d : n.decls)
      dispatch(*d);
    for (auto &s : n.body)
      dispatch(*s);
    n.validate();
  }

  void visit(const Sub &n) final {
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const Ternary &n) final {
    dispatch(*n.cond);
    dispatch(*n.lhs);
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const TypeDecl &n) final {
    dispatch(*n.value);
    n.validate();
  }

  void visit(const TypeExprID &n) final {
    /* Don't descend into *n.referent because we will already validate this
     * elsewhere.
     */
    n.validate();
  }

  void visit(const Undefine &n) final {
    dispatch(*n.rhs);
    n.validate();
  }

  void visit(const VarDecl &n) final {
    if (n.type != nullptr)
      dispatch(*n.type);
    n.validate();
  }

  virtual ~Validator() { }
};

}

void validate_model(const Model &m) {
  Validator v;
  v.dispatch(m);
}

}
