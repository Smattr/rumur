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
  virtual void visit(Assert &n);
  virtual void visit(Assignment &n);
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
  virtual void visit(Geq &n);
  virtual void visit(Gt &n);
  virtual void visit(If &n);
  virtual void visit(IfClause &n);
  virtual void visit(Implication &n);
  virtual void visit(Invariant &n);
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
  virtual void visit(Property &n);
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

}
