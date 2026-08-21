#pragma once

#include "location.hh"
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <string>
#include <vector>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

// Forward declarations to avoid a circular #include
struct ExprDecl;
struct Function;
struct TypeExpr;
struct VarDecl;

struct RUMUR_API_WITH_RTTI Expr : public Node {

  Expr(const location &loc_);

  virtual Expr *clone() const = 0;

  /// is this expression a compile-time constant?
  virtual bool constant() const = 0;

  /// The type of this expression. Never returns `nullptr`.
  virtual Ptr<TypeExpr> type() const = 0;

  /// is this expression of boolean type?
  bool is_boolean() const;

  /// Evaluate this expression. This will throw `Error` if `constant()` is not
  /// true.
  virtual mpz_class constant_fold() const = 0;

  /// is this value valid to use on the LHS of an assignment?
  virtual bool is_lvalue() const;

  /// Is this value a constant (cannot be modified)? It only makes sense to ask
  /// this of expressions for which is_lvalue() returns true. For non-lvalues,
  /// this is always true.
  virtual bool is_readonly() const;

  /// get a string representation of this expression
  std::string to_string() const;

  /// dump a string representation of this expression
  virtual void to_stream(std::ostream &out) const = 0;

  /// is this expression the boolean literal “true”?
  virtual bool is_literal_true() const;

  /// is this expression the boolean literal “false”?
  virtual bool is_literal_false() const;

  /// is this expression side-effect free?
  virtual bool is_pure() const = 0;

protected:
  Expr(const Expr &) = default;
  Expr &operator=(const Expr &) = default;
};

struct RUMUR_API_WITH_RTTI Ternary : public Expr {

  Ptr<Expr> cond;
  Ptr<Expr> lhs;
  Ptr<Expr> rhs;

  Ternary(const Ptr<Expr> &cond_, const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
          const location &loc_);
  Ternary *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;

  /* Note we do not override is_lvalue. Unlike in C, ternary expressions are not
   * considered lvalues.
   */
};

struct RUMUR_API_WITH_RTTI BinaryExpr : public Expr {

  Ptr<Expr> lhs;
  Ptr<Expr> rhs;

  BinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
             const location &loc_);

  BinaryExpr *clone() const override = 0;
  bool constant() const override;
  bool is_pure() const override;

protected:
  BinaryExpr(const BinaryExpr &) = default;
  BinaryExpr &operator=(const BinaryExpr &) = default;
};

struct RUMUR_API_WITH_RTTI BooleanBinaryExpr : public BinaryExpr {

  BooleanBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                    const location &loc_);
  BooleanBinaryExpr() = delete;

  Ptr<TypeExpr> type() const override;
  void validate() const override;

protected:
  BooleanBinaryExpr(const BooleanBinaryExpr &) = default;
  BooleanBinaryExpr &operator=(const BooleanBinaryExpr &) = default;
};

struct RUMUR_API_WITH_RTTI Implication : public BooleanBinaryExpr {

  Implication(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
              const location &loc_);
  Implication *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

/// logical OR
struct RUMUR_API_WITH_RTTI Or : public BooleanBinaryExpr {

  Or(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Or *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

/// logical AND
struct RUMUR_API_WITH_RTTI And : public BooleanBinaryExpr {

  And(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  And *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

/// An 'x & y' expression where a decision has not yet been made as to whether
/// the '&' is a logical AND or a bitwise AND. These nodes can only occur in the
/// AST prior to symbol resolution.
struct RUMUR_API_WITH_RTTI AmbiguousAmp : public BinaryExpr {

  AmbiguousAmp(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
               const location &loc_);
  AmbiguousAmp *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

/// An 'x | y' expression where a decision has not yet been made as to whether
/// the '|' is a logical OR or a bitwise OR. These nodes can only occur in the
/// AST prior to symbol resolution.
struct RUMUR_API_WITH_RTTI AmbiguousPipe : public BinaryExpr {

  AmbiguousPipe(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                const location &loc_);
  AmbiguousPipe *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI UnaryExpr : public Expr {

  Ptr<Expr> rhs;

  UnaryExpr(const Ptr<Expr> &rhs_, const location &loc_);
  UnaryExpr *clone() const override = 0;

  bool constant() const override;
  bool is_pure() const override;

protected:
  UnaryExpr(const UnaryExpr &) = default;
  UnaryExpr &operator=(const UnaryExpr &) = default;
};

struct RUMUR_API_WITH_RTTI Not : public UnaryExpr {

  Not(const Ptr<Expr> &rhs_, const location &loc_);
  Not *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI ComparisonBinaryExpr : public BinaryExpr {

  ComparisonBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                       const location &loc_);

  void validate() const override;

protected:
  ComparisonBinaryExpr(const ComparisonBinaryExpr &) = default;
  ComparisonBinaryExpr &operator=(const ComparisonBinaryExpr &) = default;
};

struct RUMUR_API_WITH_RTTI Lt : public ComparisonBinaryExpr {

  Lt(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Lt *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Leq : public ComparisonBinaryExpr {

  Leq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Leq *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Gt : public ComparisonBinaryExpr {

  Gt(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Gt *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Geq : public ComparisonBinaryExpr {

  Geq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Geq *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI EquatableBinaryExpr : public BinaryExpr {

  EquatableBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                      const location &loc_);

  void validate() const override;

protected:
  EquatableBinaryExpr(const EquatableBinaryExpr &) = default;
  EquatableBinaryExpr &operator=(const EquatableBinaryExpr &) = default;
};

struct RUMUR_API_WITH_RTTI Eq : public EquatableBinaryExpr {

  Eq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Eq *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Neq : public EquatableBinaryExpr {

  Neq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Neq *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI ArithmeticBinaryExpr : public BinaryExpr {

  ArithmeticBinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                       const location &loc_);

  Ptr<TypeExpr> type() const override;
  void validate() const override;

protected:
  ArithmeticBinaryExpr(const ArithmeticBinaryExpr &) = default;
  ArithmeticBinaryExpr &operator=(const ArithmeticBinaryExpr &) = default;
};

struct RUMUR_API_WITH_RTTI Add : public ArithmeticBinaryExpr {

  Add(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Add *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Sub : public ArithmeticBinaryExpr {

  Sub(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Sub *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Negative : public UnaryExpr {

  Negative(const Ptr<Expr> &rhs_, const location &loc_);
  Negative *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Bnot : public UnaryExpr {

  Bnot(const Ptr<Expr> &rhs_, const location &loc_);
  Bnot *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Mul : public ArithmeticBinaryExpr {

  Mul(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Mul *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Div : public ArithmeticBinaryExpr {

  Div(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Div *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Mod : public ArithmeticBinaryExpr {

  Mod(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Mod *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Lsh : public ArithmeticBinaryExpr {

  Lsh(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Lsh *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Rsh : public ArithmeticBinaryExpr {

  Rsh(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Rsh *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

/// bitwise AND
struct RUMUR_API_WITH_RTTI Band : public ArithmeticBinaryExpr {

  Band(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Band *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

/// bitwise OR
struct RUMUR_API_WITH_RTTI Bor : public ArithmeticBinaryExpr {

  Bor(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Bor *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI Xor : public ArithmeticBinaryExpr {

  Xor(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_);
  Xor *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI ExprID : public Expr {

  std::string id;
  Ptr<ExprDecl> value;

  ExprID(const std::string &id_, const Ptr<ExprDecl> &value_,
         const location &loc_);
  ExprID *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  bool is_lvalue() const override;
  bool is_readonly() const override;
  void to_stream(std::ostream &out) const override;
  bool is_literal_true() const override;
  bool is_literal_false() const override;
  bool is_pure() const override;
};

struct RUMUR_API_WITH_RTTI Field : public Expr {

  Ptr<Expr> record;
  std::string field;

  Field(const Ptr<Expr> &record_, const std::string &field_,
        const location &loc_);
  Field *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  bool is_lvalue() const override;
  bool is_readonly() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;
};

struct RUMUR_API_WITH_RTTI Element : public Expr {

  Ptr<Expr> array;
  Ptr<Expr> index;

  Element(const Ptr<Expr> &array_, const Ptr<Expr> &index_,
          const location &loc_);
  Element *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  bool is_lvalue() const override;
  bool is_readonly() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;
};

struct RUMUR_API_WITH_RTTI FunctionCall : public Expr {

  std::string name;
  Ptr<Function> function;
  std::vector<Ptr<Expr>> arguments;

  /// whether this is a child of a `ProcedureCall`
  bool within_procedure_call = false;

  FunctionCall(const std::string &name_,
               const std::vector<Ptr<Expr>> &arguments_, const location &loc_);
  FunctionCall *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;
};

struct RUMUR_API_WITH_RTTI Quantifier : public Node {

  std::string name;

  // if this is != nullptr, the from/to/step will be nullptr
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
  Quantifier *clone() const override;
  void validate() const override;
  std::string to_string() const;
  void to_stream(std::ostream &out) const;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  /// whether the quantifier’s range can be constant folded
  bool constant() const;

  /// number of entries in this quantifier’s range (only valid when
  /// `constant()` returns `true`)
  mpz_class count() const;

  /// get the lower bound of this quantified expression as a C expression
  std::string lower_bound() const;

  /// is this side-effect free?
  bool is_pure() const;
};

struct RUMUR_API_WITH_RTTI Exists : public Expr {

  Quantifier quantifier;
  Ptr<Expr> expr;

  Exists(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
         const location &loc_);
  Exists *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;
};

struct RUMUR_API_WITH_RTTI Forall : public Expr {

  Quantifier quantifier;
  Ptr<Expr> expr;

  Forall(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
         const location &loc_);
  Forall *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;
};

struct RUMUR_API_WITH_RTTI IsMember : public Expr {

  Ptr<Expr> peg;
  Ptr<TypeExpr> hole;

  IsMember(const Ptr<Expr> &peg_, const Ptr<TypeExpr> &hole_,
           const location &loc_);
  IsMember *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;
};

struct RUMUR_API_WITH_RTTI IsUndefined : public UnaryExpr {

  IsUndefined(const Ptr<Expr> &expr_, const location &loc_);
  IsUndefined *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
};

struct RUMUR_API_WITH_RTTI MultisetCount : public Expr {
  std::string identifier;
  Ptr<Expr> container;
  Ptr<Expr> predicate;

  MultisetCount(const std::string &identifier_, const Ptr<Expr> &container_,
                const Ptr<Expr> &predicate_, const location &loc_);
  MultisetCount *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  bool constant() const override;
  Ptr<TypeExpr> type() const override;
  mpz_class constant_fold() const override;
  void validate() const override;
  void to_stream(std::ostream &out) const override;
  bool is_pure() const override;
};

} // namespace rumur
