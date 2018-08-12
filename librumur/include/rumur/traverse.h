#pragma once

#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>

namespace rumur {

/* Generic abstract syntax tree traversal. To perform some action on the nodes
 * of a model, you should implement a class that inherits from this one and
 * override whichever 'visit' alternatives are relevant for your situation.
 */
class Traversal {

 public:
  virtual void visit(Add &n);
  virtual void visit(And &n);
  virtual void visit(Array &n);
  virtual void visit(Assignment &n);
  virtual void visit(Clear &n);
  virtual void visit(ConstDecl &n);
  virtual void visit(Div &n);
  virtual void visit(Element &n);
  virtual void visit(Enum&);
  virtual void visit(Eq &n);
  virtual void visit(ErrorStmt&);
  virtual void visit(Exists &n);
  virtual void visit(ExprID &n);
  virtual void visit(Field &n);
  virtual void visit(For &n);
  virtual void visit(Forall &n);
  virtual void visit(Function &n);
  virtual void visit(FunctionCall &n);
  virtual void visit(Geq &n);
  virtual void visit(Gt &n);
  virtual void visit(If &n);
  virtual void visit(IfClause &n);
  virtual void visit(Implication &n);
  virtual void visit(Leq &n);
  virtual void visit(Lt &n);
  virtual void visit(Model &n);
  virtual void visit(Mod &n);
  virtual void visit(Mul &n);
  virtual void visit(Negative &n);
  virtual void visit(Neq &n);
  virtual void visit(Not &n);
  virtual void visit(Number &n);
  virtual void visit(Or &n);
  virtual void visit(Parameter &n);
  virtual void visit(Property &n);
  virtual void visit(PropertyRule &n);
  virtual void visit(PropertyStmt &n);
  virtual void visit(Quantifier &n);
  virtual void visit(Range &n);
  virtual void visit(Record &n);
  virtual void visit(Return &n);
  virtual void visit(Ruleset &n);
  virtual void visit(Scalarset &n);
  virtual void visit(SimpleRule &n);
  virtual void visit(StartState &n);
  virtual void visit(Sub &n);
  virtual void visit(Ternary &n);
  virtual void visit(TypeDecl &n);
  virtual void visit(TypeExprID &n);
  virtual void visit(Undefine &n);
  virtual void visit(VarDecl &n);

  /* Visitation dispatch. This simply determines the type of the Node argument
   * and calls the appropriate specialised 'visit' method. This is not virtual
   * because we do not anticipate use cases where this behaviour needs to
   * change.
   */
  void dispatch(Node &n);

  // Force class to be abstract
  virtual ~Traversal() = 0;

 private:
  void visit(BinaryExpr &n);
  void visit(UnaryExpr &n);
};

// Read-only equivalent of Traversal.
class ConstTraversal {

 public:
  virtual void visit(const Add &n);
  virtual void visit(const And &n);
  virtual void visit(const Array &n);
  virtual void visit(const Assignment &n);
  virtual void visit(const Clear &n);
  virtual void visit(const ConstDecl &n);
  virtual void visit(const Div &n);
  virtual void visit(const Element &n);
  virtual void visit(const Enum&);
  virtual void visit(const Eq &n);
  virtual void visit(const ErrorStmt&);
  virtual void visit(const Exists &n);
  virtual void visit(const ExprID &n);
  virtual void visit(const Field &n);
  virtual void visit(const For &n);
  virtual void visit(const Forall &n);
  virtual void visit(const Function &n);
  virtual void visit(const FunctionCall &n);
  virtual void visit(const Geq &n);
  virtual void visit(const Gt &n);
  virtual void visit(const If &n);
  virtual void visit(const IfClause &n);
  virtual void visit(const Implication &n);
  virtual void visit(const Leq &n);
  virtual void visit(const Lt &n);
  virtual void visit(const Model &n);
  virtual void visit(const Mod &n);
  virtual void visit(const Mul &n);
  virtual void visit(const Negative &n);
  virtual void visit(const Neq &n);
  virtual void visit(const Not &n);
  virtual void visit(const Number &n);
  virtual void visit(const Or &n);
  virtual void visit(const Parameter &n);
  virtual void visit(const Property &n);
  virtual void visit(const PropertyRule &n);
  virtual void visit(const PropertyStmt &n);
  virtual void visit(const Quantifier &n);
  virtual void visit(const Range &n);
  virtual void visit(const Record &n);
  virtual void visit(const Return &n);
  virtual void visit(const Ruleset &n);
  virtual void visit(const Scalarset &n);
  virtual void visit(const SimpleRule &n);
  virtual void visit(const StartState &n);
  virtual void visit(const Sub &n);
  virtual void visit(const Ternary &n);
  virtual void visit(const TypeDecl &n);
  virtual void visit(const TypeExprID &n);
  virtual void visit(const Undefine &n);
  virtual void visit(const VarDecl &n);

  void dispatch(const Node &n);

  // Force class to be abstract
  virtual ~ConstTraversal() = 0;

 private:
  void visit(const BinaryExpr &n);
  void visit(const UnaryExpr &n);
};

}
