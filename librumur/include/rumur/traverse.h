#pragma once

#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>

namespace rumur {

/* Generic abstract syntax tree traversal interface with no implementation. If
 * you want to implement a traversal that handles every Node type you should
 * define a class that inherits from this. If you want a more liberal base class
 * that provides default implementations for the 'visit' methods you don't need
 * to override, inherit from Traversal below.
 */
class BaseTraversal {

 public:
  virtual void visit(Add &n) = 0;
  virtual void visit(And &n) = 0;
  virtual void visit(Array &n) = 0;
  virtual void visit(Assignment &n) = 0;
  virtual void visit(Clear &n) = 0;
  virtual void visit(ConstDecl &n) = 0;
  virtual void visit(Div &n) = 0;
  virtual void visit(Element &n) = 0;
  virtual void visit(Enum&) = 0;
  virtual void visit(Eq &n) = 0;
  virtual void visit(ErrorStmt&) = 0;
  virtual void visit(Exists &n) = 0;
  virtual void visit(ExprID &n) = 0;
  virtual void visit(Field &n) = 0;
  virtual void visit(For &n) = 0;
  virtual void visit(Forall &n) = 0;
  virtual void visit(Function &n) = 0;
  virtual void visit(FunctionCall &n) = 0;
  virtual void visit(Geq &n) = 0;
  virtual void visit(Gt &n) = 0;
  virtual void visit(If &n) = 0;
  virtual void visit(IfClause &n) = 0;
  virtual void visit(Implication &n) = 0;
  virtual void visit(Leq &n) = 0;
  virtual void visit(Lt &n) = 0;
  virtual void visit(Model &n) = 0;
  virtual void visit(Mod &n) = 0;
  virtual void visit(Mul &n) = 0;
  virtual void visit(Negative &n) = 0;
  virtual void visit(Neq &n) = 0;
  virtual void visit(Not &n) = 0;
  virtual void visit(Number &n) = 0;
  virtual void visit(Or &n) = 0;
  virtual void visit(Parameter &n) = 0;
  virtual void visit(ProcedureCall &n) = 0;
  virtual void visit(Property &n) = 0;
  virtual void visit(PropertyRule &n) = 0;
  virtual void visit(PropertyStmt &n) = 0;
  virtual void visit(Quantifier &n) = 0;
  virtual void visit(Range &n) = 0;
  virtual void visit(Record &n) = 0;
  virtual void visit(Return &n) = 0;
  virtual void visit(Ruleset &n) = 0;
  virtual void visit(Scalarset &n) = 0;
  virtual void visit(SimpleRule &n) = 0;
  virtual void visit(StartState &n) = 0;
  virtual void visit(Sub &n) = 0;
  virtual void visit(Ternary &n) = 0;
  virtual void visit(TypeDecl &n) = 0;
  virtual void visit(TypeExprID &n) = 0;
  virtual void visit(Undefine &n) = 0;
  virtual void visit(VarDecl &n) = 0;

  /* Visitation dispatch. This simply determines the type of the Node argument
   * and calls the appropriate specialised 'visit' method. This is not virtual
   * because we do not anticipate use cases where this behaviour needs to
   * change.
   */
  void dispatch(Node &n);

  virtual ~BaseTraversal() { }
};

/* Generic abstract syntax tree traversal. To perform some action on the nodes
 * of a model, you should implement a class that inherits from this one and
 * override whichever 'visit' alternatives are relevant for your situation.
 */
class Traversal : public BaseTraversal {

 public:
  void visit(Add &n) override;
  void visit(And &n) override;
  void visit(Array &n) override;
  void visit(Assignment &n) override;
  void visit(Clear &n) override;
  void visit(ConstDecl &n) override;
  void visit(Div &n) override;
  void visit(Element &n) override;
  void visit(Enum&) override;
  void visit(Eq &n) override;
  void visit(ErrorStmt&) override;
  void visit(Exists &n) override;
  void visit(ExprID &n) override;
  void visit(Field &n) override;
  void visit(For &n) override;
  void visit(Forall &n) override;
  void visit(Function &n) override;
  void visit(FunctionCall &n) override;
  void visit(Geq &n) override;
  void visit(Gt &n) override;
  void visit(If &n) override;
  void visit(IfClause &n) override;
  void visit(Implication &n) override;
  void visit(Leq &n) override;
  void visit(Lt &n) override;
  void visit(Model &n) override;
  void visit(Mod &n) override;
  void visit(Mul &n) override;
  void visit(Negative &n) override;
  void visit(Neq &n) override;
  void visit(Not &n) override;
  void visit(Number &n) override;
  void visit(Or &n) override;
  void visit(Parameter &n) override;
  void visit(ProcedureCall &n) override;
  void visit(Property &n) override;
  void visit(PropertyRule &n) override;
  void visit(PropertyStmt &n) override;
  void visit(Quantifier &n) override;
  void visit(Range &n) override;
  void visit(Record &n) override;
  void visit(Return &n) override;
  void visit(Ruleset &n) override;
  void visit(Scalarset &n) override;
  void visit(SimpleRule &n) override;
  void visit(StartState &n) override;
  void visit(Sub &n) override;
  void visit(Ternary &n) override;
  void visit(TypeDecl &n) override;
  void visit(TypeExprID &n) override;
  void visit(Undefine &n) override;
  void visit(VarDecl &n) override;

  // Force class to be abstract
  virtual ~Traversal() = 0;

 private:
  void visit(BinaryExpr &n);
  void visit(UnaryExpr &n);
};

// Read-only equivalent of BaseTraversal.
class ConstBaseTraversal {

 public:
  virtual void visit(const Add &n) = 0;
  virtual void visit(const And &n) = 0;
  virtual void visit(const Array &n) = 0;
  virtual void visit(const Assignment &n) = 0;
  virtual void visit(const Clear &n) = 0;
  virtual void visit(const ConstDecl &n) = 0;
  virtual void visit(const Div &n) = 0;
  virtual void visit(const Element &n) = 0;
  virtual void visit(const Enum&) = 0;
  virtual void visit(const Eq &n) = 0;
  virtual void visit(const ErrorStmt&) = 0;
  virtual void visit(const Exists &n) = 0;
  virtual void visit(const ExprID &n) = 0;
  virtual void visit(const Field &n) = 0;
  virtual void visit(const For &n) = 0;
  virtual void visit(const Forall &n) = 0;
  virtual void visit(const Function &n) = 0;
  virtual void visit(const FunctionCall &n) = 0;
  virtual void visit(const Geq &n) = 0;
  virtual void visit(const Gt &n) = 0;
  virtual void visit(const If &n) = 0;
  virtual void visit(const IfClause &n) = 0;
  virtual void visit(const Implication &n) = 0;
  virtual void visit(const Leq &n) = 0;
  virtual void visit(const Lt &n) = 0;
  virtual void visit(const Model &n) = 0;
  virtual void visit(const Mod &n) = 0;
  virtual void visit(const Mul &n) = 0;
  virtual void visit(const Negative &n) = 0;
  virtual void visit(const Neq &n) = 0;
  virtual void visit(const Not &n) = 0;
  virtual void visit(const Number &n) = 0;
  virtual void visit(const Or &n) = 0;
  virtual void visit(const Parameter &n) = 0;
  virtual void visit(const ProcedureCall &n) = 0;
  virtual void visit(const Property &n) = 0;
  virtual void visit(const PropertyRule &n) = 0;
  virtual void visit(const PropertyStmt &n) = 0;
  virtual void visit(const Quantifier &n) = 0;
  virtual void visit(const Range &n) = 0;
  virtual void visit(const Record &n) = 0;
  virtual void visit(const Return &n) = 0;
  virtual void visit(const Ruleset &n) = 0;
  virtual void visit(const Scalarset &n) = 0;
  virtual void visit(const SimpleRule &n) = 0;
  virtual void visit(const StartState &n) = 0;
  virtual void visit(const Sub &n) = 0;
  virtual void visit(const Ternary &n) = 0;
  virtual void visit(const TypeDecl &n) = 0;
  virtual void visit(const TypeExprID &n) = 0;
  virtual void visit(const Undefine &n) = 0;
  virtual void visit(const VarDecl &n) = 0;

  void dispatch(const Node &n);

  virtual ~ConstBaseTraversal() { }
};


// Read-only equivalent of Traversal.
class ConstTraversal : public ConstBaseTraversal {

 public:
  void visit(const Add &n) override;
  void visit(const And &n) override;
  void visit(const Array &n) override;
  void visit(const Assignment &n) override;
  void visit(const Clear &n) override;
  void visit(const ConstDecl &n) override;
  void visit(const Div &n) override;
  void visit(const Element &n) override;
  void visit(const Enum&) override;
  void visit(const Eq &n) override;
  void visit(const ErrorStmt&) override;
  void visit(const Exists &n) override;
  void visit(const ExprID &n) override;
  void visit(const Field &n) override;
  void visit(const For &n) override;
  void visit(const Forall &n) override;
  void visit(const Function &n) override;
  void visit(const FunctionCall &n) override;
  void visit(const Geq &n) override;
  void visit(const Gt &n) override;
  void visit(const If &n) override;
  void visit(const IfClause &n) override;
  void visit(const Implication &n) override;
  void visit(const Leq &n) override;
  void visit(const Lt &n) override;
  void visit(const Model &n) override;
  void visit(const Mod &n) override;
  void visit(const Mul &n) override;
  void visit(const Negative &n) override;
  void visit(const Neq &n) override;
  void visit(const Not &n) override;
  void visit(const Number &n) override;
  void visit(const Or &n) override;
  void visit(const Parameter &n) override;
  void visit(const ProcedureCall &n) override;
  void visit(const Property &n) override;
  void visit(const PropertyRule &n) override;
  void visit(const PropertyStmt &n) override;
  void visit(const Quantifier &n) override;
  void visit(const Range &n) override;
  void visit(const Record &n) override;
  void visit(const Return &n) override;
  void visit(const Ruleset &n) override;
  void visit(const Scalarset &n) override;
  void visit(const SimpleRule &n) override;
  void visit(const StartState &n) override;
  void visit(const Sub &n) override;
  void visit(const Ternary &n) override;
  void visit(const TypeDecl &n) override;
  void visit(const TypeExprID &n) override;
  void visit(const Undefine &n) override;
  void visit(const VarDecl &n) override;

  // Force class to be abstract
  virtual ~ConstTraversal() = 0;

 private:
  void visit(const BinaryExpr &n);
  void visit(const UnaryExpr &n);
};

}
