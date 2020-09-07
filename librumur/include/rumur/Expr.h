#pragma once

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <string>
#include <vector>

namespace rumur {

// Forward declarations to avoid a circular #include
struct ExprDecl;
struct Function;
struct TypeExpr;
struct VarDecl;

struct Expr : public Node {

  Expr(const location &loc_);
  virtual ~Expr() = default;

  virtual Expr *clone() const = 0;

  // Whether an expression is a compile-time constant
  virtual bool constant() const = 0;

  /* The type of this expression. A nullptr indicates the type is equivalent
   * to a numeric literal; that is, an unbounded range.
   */
  virtual Ptr<TypeExpr> type() const = 0;

  // If this expression is of boolean type.
  bool is_boolean() const;

  virtual mpz_class constant_fold() const = 0;

  // Is this value valid to use on the LHS of an assignment?
  virtual bool is_lvalue() const;

  /* Is this value a constant (cannot be modified)? It only makes sense to ask
   * this of expressions for which is_lvalue() returns true. For non-lvalues,
   * this is always true.
   */
  virtual bool is_readonly() const;

  // Get a string representation of this expression
  virtual std::string to_string() const = 0;

  // is this expression the boolean literal “true”?
  virtual bool is_literal_true() const;

  // is this expression the boolean literal “false”?
  virtual bool is_literal_false() const;

  // is this expression side-effect free?
  virtual bool is_pure() const = 0;
};

struct Ternary : public Expr {

  Ptr<Expr> cond;
  Ptr<Expr> lhs;
  Ptr<Expr> rhs;

  Ternary(const Ptr<Expr> &cond_, const Ptr<Expr> &lhs_,
    const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Ternary() = default;
  Ternary *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
  bool is_pure() const final;

  /* Note we do not override is_lvalue. Unlike in C, ternary expressions are not
   * considered lvalues.
   */
};

struct BinaryExpr : public Expr {

  Ptr<Expr> lhs;
  Ptr<Expr> rhs;

  BinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);
  virtual ~BinaryExpr() = default;

  BinaryExpr *clone() const override = 0;
  bool constant() const final;
  bool is_pure() const final;
};

struct BooleanBinaryExpr : public BinaryExpr {

  BooleanBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);
  BooleanBinaryExpr() = delete;

  Ptr<TypeExpr> type() const final;
  void validate() const final;
};

struct Implication : public BooleanBinaryExpr {

  Implication(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);
  Implication *clone() const final;
  virtual ~Implication() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

// logical OR
struct Or : public BooleanBinaryExpr {

  Or(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Or() = default;
  Or *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

// logical AND
struct And : public BooleanBinaryExpr {

  And(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~And() = default;
  And *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

// An 'x & y' expression where a decision has not yet been made as to whether
// the '&' is a logical AND or a bitwise AND. These nodes can only occur in the
// AST prior to symbol resolution.
struct AmbiguousAmp : public BinaryExpr {

  AmbiguousAmp(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~AmbiguousAmp() = default;
  AmbiguousAmp *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

// An 'x | y' expression where a decision has not yet been made as to whether
// the '|' is a logical OR or a bitwise OR. These nodes can only occur in the
// AST prior to symbol resolution.
struct AmbiguousPipe : public BinaryExpr {

  AmbiguousPipe(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);
  virtual ~AmbiguousPipe() = default;
  AmbiguousPipe *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct UnaryExpr : public Expr {

  Ptr<Expr> rhs;

  UnaryExpr(const Ptr<Expr> &rhs_, const location &loc_);
  UnaryExpr *clone() const override = 0;
  virtual ~UnaryExpr() = default;

  bool constant() const override;
  bool is_pure() const final;
};

struct Not : public UnaryExpr {

  Not(const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Not() = default;
  Not *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
};

struct ComparisonBinaryExpr : public BinaryExpr {

  ComparisonBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);

  void validate() const final;
};

struct Lt : public ComparisonBinaryExpr {

  Lt(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Lt() = default;
  Lt *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Leq : public ComparisonBinaryExpr {

  Leq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Leq() = default;
  Leq *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Gt : public ComparisonBinaryExpr {

  Gt(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Gt() = default;
  Gt *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Geq : public ComparisonBinaryExpr {

  Geq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Geq() = default;
  Geq *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct EquatableBinaryExpr : public BinaryExpr {

  EquatableBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);

  void validate() const final;
};

struct Eq : public EquatableBinaryExpr {

  Eq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Eq() = default;
  Eq *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Neq : public EquatableBinaryExpr {

  Neq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Neq() = default;
  Neq *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct ArithmeticBinaryExpr : public BinaryExpr {

  ArithmeticBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);

  Ptr<TypeExpr> type() const final;
  void validate() const final;
};

struct Add : public ArithmeticBinaryExpr {

  Add(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Add() = default;
  Add *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Sub : public ArithmeticBinaryExpr {

  Sub(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Sub() = default;
  Sub *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Negative : public UnaryExpr {

  Negative(const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Negative() = default;
  Negative *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
};

struct Bnot : public UnaryExpr {

  Bnot(const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Bnot() = default;
  Bnot *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
};

struct Mul : public ArithmeticBinaryExpr {

  Mul(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Mul() = default;
  Mul *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Div : public ArithmeticBinaryExpr {

  Div(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Div() = default;
  Div *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Mod : public ArithmeticBinaryExpr {

  Mod(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Mod() = default;
  Mod *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Lsh : public ArithmeticBinaryExpr {

  Lsh(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Lsh() = default;
  Lsh *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Rsh : public ArithmeticBinaryExpr {

  Rsh(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Rsh() = default;
  Rsh *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

// bitwise AND
struct Band : public ArithmeticBinaryExpr {

  Band(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Band() = default;
  Band *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

// bitwise OR
struct Bor : public ArithmeticBinaryExpr {

  Bor(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Bor() = default;
  Bor *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};

struct Xor : public ArithmeticBinaryExpr {

  Xor(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Xor() = default;
  Xor *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class constant_fold() const final;
  std::string to_string() const final;
};


struct ExprID : public Expr {

  std::string id;
  Ptr<ExprDecl> value;

  ExprID(const std::string &id_, const Ptr<ExprDecl> &value_,
    const location &loc_);
  virtual ~ExprID() = default;
  ExprID *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  bool is_lvalue() const final;
  bool is_readonly() const final;
  std::string to_string() const final;
  bool is_literal_true() const final;
  bool is_literal_false() const final;
  bool is_pure() const final;
};

struct Field : public Expr {

  Ptr<Expr> record;
  std::string field;

  Field(const Ptr<Expr> &record_, const std::string &field_,
    const location &loc_);
  virtual ~Field() = default;
  Field *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  bool is_lvalue() const final;
  bool is_readonly() const final;
  std::string to_string() const final;
  bool is_pure() const final;
};

struct Element : public Expr {

  Ptr<Expr> array;
  Ptr<Expr> index;

  Element(const Ptr<Expr> &array_, const Ptr<Expr> &index_,
    const location &loc_);
  virtual ~Element() = default;
  Element *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  bool is_lvalue() const final;
  bool is_readonly() const final;
  std::string to_string() const final;
  bool is_pure() const final;
};

struct FunctionCall : public Expr {

  std::string name;
  Ptr<Function> function;
  std::vector<Ptr<Expr>> arguments;

  // Whether this is a child of a ProcedureCall
  bool within_procedure_call = false;

  FunctionCall(const std::string &name_,
    const std::vector<Ptr<Expr>> &arguments_, const location &loc_);
  virtual ~FunctionCall() = default;
  FunctionCall *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
  bool is_pure() const final;
};

struct Quantifier : public Node {

  std::string name;

  // If this is != nullptr, the from/to/step will be nullptr
  Ptr<TypeExpr> type;

  Ptr<Expr> from;
  Ptr<Expr> to;
  Ptr<Expr> step;

  Ptr<VarDecl> decl;

  Quantifier(const std::string &name_, const Ptr<TypeExpr> &type_,
    const location &loc);
  Quantifier(const std::string &name_, const Ptr<Expr> &from_,
    const Ptr<Expr> &to_, const location &loc_);
  Quantifier(const std::string &name_, const Ptr<Expr> &from_,
    const Ptr<Expr> &to_, const Ptr<Expr> &step_,
    const location &loc_);
  virtual ~Quantifier() = default;
  Quantifier *clone() const final;
  void validate() const final;
  std::string to_string() const;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  // whether the quantifier's range can be constant folded
  bool constant() const;

  /* number of entries in this quantifier's range (only valid when constant()
   * returns true)
   */
  mpz_class count() const;

  // get the lower bound of this quantified expression as a C expression
  std::string lower_bound() const;

  // is this side-effect free?
  bool is_pure() const;
};

struct Exists : public Expr {

  Quantifier quantifier;
  Ptr<Expr> expr;

  Exists(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
    const location &loc_);
  virtual ~Exists() = default;
  Exists *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
  bool is_pure() const final;
};

struct Forall : public Expr {

  Quantifier quantifier;
  Ptr<Expr> expr;

  Forall(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
    const location &loc_);
  virtual ~Forall() = default;
  Forall *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
  bool is_pure() const final;
};

struct IsUndefined : public UnaryExpr {

  IsUndefined(const Ptr<Expr> &expr_, const location &loc_);
  virtual ~IsUndefined() = default;
  IsUndefined *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool constant() const final;
  Ptr<TypeExpr> type() const final;
  mpz_class constant_fold() const final;
  void validate() const final;
  std::string to_string() const final;
};

}
