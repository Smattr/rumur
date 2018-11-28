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

  using Node::Node;
  virtual ~Expr() = default;

  virtual Expr *clone() const = 0;

  // Whether an expression is a compile-time constant
  virtual bool constant() const = 0;

  /* The type of this expression. A nullptr indicates the type is equivalent
   * to a numeric literal; that is, an unbounded range.
   */
  virtual const TypeExpr *type() const = 0;

  // If this expression is of boolean type.
  bool is_boolean() const;

  virtual mpz_class constant_fold() const = 0;

  // Is this value valid to use on the LHS of an assignment?
  virtual bool is_lvalue() const;

  // Get a string representation of this expression
  virtual std::string to_string() const = 0;
};

struct Ternary : public Expr {

  Ptr<Expr> cond;
  Ptr<Expr> lhs;
  Ptr<Expr> rhs;

  Ternary(const Ptr<Expr> &cond_, const Ptr<Expr> &lhs_,
    const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Ternary() = default;

  Ternary *clone() const final;
  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  std::string to_string() const final;

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
};

struct BooleanBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  BooleanBinaryExpr() = delete;

  void validate() const final;
};

struct Implication : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  Implication *clone() const final;
  virtual ~Implication() = default;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Or : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  virtual ~Or() = default;
  Or *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct And : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  virtual ~And() = default;
  And *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct UnaryExpr : public Expr {

  Ptr<Expr> rhs;

  UnaryExpr(const Ptr<Expr> &rhs_, const location &loc_);
  UnaryExpr *clone() const override = 0;
  virtual ~UnaryExpr() = default;

  bool constant() const final;
};

struct Not : public UnaryExpr {

  using UnaryExpr::UnaryExpr;
  virtual ~Not() = default;
  Not *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  std::string to_string() const final;
};

struct ComparisonBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;

  void validate() const final;
};

struct Lt : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  virtual ~Lt() = default;
  Lt *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Leq : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  virtual ~Leq() = default;
  Leq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Gt : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  virtual ~Gt() = default;
  Gt *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Geq : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  virtual ~Geq() = default;
  Geq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct EquatableBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;

  void validate() const final;
};

struct Eq : public EquatableBinaryExpr {

  using EquatableBinaryExpr::EquatableBinaryExpr;
  virtual ~Eq() = default;
  Eq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Neq : public EquatableBinaryExpr {

  using EquatableBinaryExpr::EquatableBinaryExpr;
  virtual ~Neq() = default;
  Neq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct ArithmeticBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;

  void validate() const final;
};

struct Add : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  virtual ~Add() = default;
  Add *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Sub : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  virtual ~Sub() = default;
  Sub *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Negative : public UnaryExpr {

  using UnaryExpr::UnaryExpr;
  virtual ~Negative() = default;
  Negative *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  std::string to_string() const final;
};

struct Mul : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  virtual ~Mul() = default;
  Mul *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Div : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  virtual ~Div() = default;
  Div *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct Mod : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  virtual ~Mod() = default;
  Mod *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const final;
};

struct ExprID : public Expr {

  std::string id;
  Ptr<ExprDecl> value;

  ExprID(const std::string &id_, const Ptr<ExprDecl> &value_,
    const location &loc_);
  virtual ~ExprID() = default;
  ExprID *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  bool is_lvalue() const final;
  std::string to_string() const final;
};

struct Field : public Expr {

  Ptr<Expr> record;
  std::string field;

  Field(const Ptr<Expr> &record_, const std::string &field_,
    const location &loc_);
  virtual ~Field() = default;
  Field *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  bool is_lvalue() const final;
  std::string to_string() const final;
};

struct Element : public Expr {

  Ptr<Expr> array;
  Ptr<Expr> index;

  Element(const Ptr<Expr> &array_, const Ptr<Expr> &index_,
    const location &loc_);
  virtual ~Element() = default;
  Element *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  bool is_lvalue() const final;
  std::string to_string() const final;
};

struct FunctionCall : public Expr {

  std::string name;
  Ptr<Function> function;
  std::vector<Ptr<Expr>> arguments;

  FunctionCall(const std::string &name_,
    const std::vector<Ptr<Expr>> &arguments_, const location &loc_);
  virtual ~FunctionCall() = default;
  FunctionCall *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  std::string to_string() const final;
};

struct Quantifier : public Node {

  std::string name;

  // If this is != nullptr, the from/to/step will be nullptr
  Ptr<TypeExpr> type;

  Ptr<Expr> from;
  Ptr<Expr> to;
  Ptr<Expr> step;

  Quantifier(const std::string &name_, const Ptr<TypeExpr> &type_,
    const location &loc);
  Quantifier(const std::string &name_, const Ptr<Expr> &from_,
    const Ptr<Expr> &to_, const location &loc_);
  Quantifier(const std::string &name_, const Ptr<Expr> &from_,
    const Ptr<Expr> &to_, const Ptr<Expr> &step_,
    const location &loc_);
  virtual ~Quantifier() = default;
  Quantifier *clone() const final;
  bool operator==(const Node &other) const final;
  std::string to_string() const;
};

struct Exists : public Expr {

  Quantifier quantifier;
  Ptr<Expr> expr;

  Exists(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
    const location &loc_);
  virtual ~Exists() = default;
  Exists *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  std::string to_string() const final;
};

struct Forall : public Expr {

  Quantifier quantifier;
  Ptr<Expr> expr;

  Forall(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
    const location &loc_);
  virtual ~Forall() = default;
  Forall *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  std::string to_string() const final;
};

}
