#pragma once

#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Node.h>
#include <string>
#include <vector>

namespace rumur {

// Forward declarations to avoid a circular #include
struct Decl;
struct Function;
struct TypeExpr;
struct VarDecl;

struct Expr : public Node {

  using Node::Node;
  Expr() = delete;
  Expr(const Expr&) = default;
  Expr(Expr&&) = default;
  Expr &operator=(const Expr&) = default;
  Expr &operator=(Expr&&) = default;
  virtual ~Expr() { }

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
};

struct Ternary : public Expr {

  std::shared_ptr<Expr> cond;
  std::shared_ptr<Expr> lhs;
  std::shared_ptr<Expr> rhs;

  Ternary() = delete;
  Ternary(std::shared_ptr<Expr> cond_, std::shared_ptr<Expr> lhs_,
    std::shared_ptr<Expr> rhs_, const location &loc_);
  Ternary(const Ternary &other);
  Ternary(Ternary&&) = default;
  Ternary &operator=(Ternary other);
  friend void swap(Ternary &x, Ternary &y) noexcept;
  virtual ~Ternary() { }

  Ternary *clone() const final;
  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;

  /* Note we do not override is_lvalue. Unlike in C, ternary expressions are not
   * considered lvalues.
   */
};

struct BinaryExpr : public Expr {

  std::shared_ptr<Expr> lhs;
  std::shared_ptr<Expr> rhs;

  BinaryExpr() = delete;
  BinaryExpr(std::shared_ptr<Expr> lhs_, std::shared_ptr<Expr> rhs_,
    const location &loc_);
  BinaryExpr(const BinaryExpr &other);
  BinaryExpr &operator=(const BinaryExpr&) = delete;
  BinaryExpr &operator=(BinaryExpr&&) = delete;
  friend void swap(BinaryExpr &x, BinaryExpr &y) noexcept;
  virtual ~BinaryExpr() { }

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
  Implication() = delete;
  Implication &operator=(Implication other);
  Implication *clone() const final;
  virtual ~Implication() { }

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Or : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  Or() = delete;
  Or &operator=(Or other);
  virtual ~Or() { }
  Or *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct And : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  And() = delete;
  And &operator=(And other);
  virtual ~And() { }
  And *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct UnaryExpr : public Expr {

  std::shared_ptr<Expr> rhs;

  UnaryExpr() = delete;
  UnaryExpr(std::shared_ptr<Expr> rhs_, const location &loc_);
  UnaryExpr(const UnaryExpr &other);
  friend void swap(UnaryExpr &x, UnaryExpr &y) noexcept;
  UnaryExpr *clone() const override = 0;
  virtual ~UnaryExpr() { }

  bool constant() const final;
};

struct Not : public UnaryExpr {

  using UnaryExpr::UnaryExpr;
  Not() = delete;
  Not &operator=(Not other);
  virtual ~Not() { }
  Not *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct ComparisonBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  ComparisonBinaryExpr() = delete;

  void validate() const final;
};

struct Lt : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Lt() = delete;
  Lt &operator=(Lt other);
  virtual ~Lt() { }
  Lt *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Leq : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Leq() = delete;
  Leq &operator=(Leq other);
  virtual ~Leq() { }
  Leq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Gt : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Gt() = delete;
  Gt &operator=(Gt other);
  virtual ~Gt() { }
  Gt *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Geq : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Geq() = delete;
  Geq &operator=(Geq other);
  virtual ~Geq() { }
  Geq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct EquatableBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  EquatableBinaryExpr() = delete;

  void validate() const final;
};

struct Eq : public EquatableBinaryExpr {

  using EquatableBinaryExpr::EquatableBinaryExpr;
  Eq() = delete;
  Eq &operator=(Eq other);
  virtual ~Eq() { }
  Eq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Neq : public EquatableBinaryExpr {

  using EquatableBinaryExpr::EquatableBinaryExpr;
  Neq() = delete;
  Neq &operator=(Neq other);
  virtual ~Neq() { }
  Neq *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct ArithmeticBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  ArithmeticBinaryExpr() = delete;

  void validate() const final;
};

struct Add : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Add() = delete;
  Add &operator=(Add other);
  virtual ~Add() { }
  Add *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Sub : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Sub() = delete;
  Sub &operator=(Sub other);
  virtual ~Sub() { }
  Sub *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Negative : public UnaryExpr {

  using UnaryExpr::UnaryExpr;
  Negative() = delete;
  Negative &operator=(Negative other);
  virtual ~Negative() { }
  Negative *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct Mul : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Mul() = delete;
  Mul &operator=(Mul other);
  virtual ~Mul() { }
  Mul *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Div : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Div() = delete;
  Div &operator=(Div other);
  virtual ~Div() { }
  Div *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Mod : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Mod() = delete;
  Mod &operator=(Mod other);
  virtual ~Mod() { }
  Mod *clone() const final;

  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct ExprID : public Expr {

  std::string id;
  std::shared_ptr<Decl> value;

  ExprID() = delete;
  ExprID(const std::string &id_, const std::shared_ptr<Decl> value_, const location &loc_);
  ExprID(const ExprID &other);
  ExprID &operator=(ExprID other);
  friend void swap(ExprID &x, ExprID &y) noexcept;
  virtual ~ExprID() { }
  ExprID *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
  bool is_lvalue() const final;
};

struct Field : public Expr {

  std::shared_ptr<Expr> record;
  std::string field;

  Field() = delete;
  Field(std::shared_ptr<Expr> record_, const std::string &field_,
    const location &loc_);
  Field(const Field &other);
  friend void swap(Field &x, Field &y) noexcept;
  Field &operator=(Field other);
  virtual ~Field() { }
  Field *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  bool is_lvalue() const final;
};

struct Element : public Expr {

  std::shared_ptr<Expr> array;
  std::shared_ptr<Expr> index;

  Element() = delete;
  Element(std::shared_ptr<Expr> array_, std::shared_ptr<Expr> index_,
    const location &loc_);
  Element(const Element &other);
  friend void swap(Element &x, Element &y) noexcept;
  Element &operator=(Element other);
  virtual ~Element() { }
  Element *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  bool is_lvalue() const final;
};

struct FunctionCall : public Expr {

  std::string name;
  std::shared_ptr<Function> function;
  std::vector<std::shared_ptr<Expr>> arguments;

  FunctionCall() = delete;
  FunctionCall(const std::string &name_, std::shared_ptr<Function> function_,
    std::vector<std::shared_ptr<Expr>> arguments_, const location &loc_);
  FunctionCall(const FunctionCall &other);
  friend void swap(FunctionCall &x, FunctionCall &y) noexcept;
  FunctionCall &operator=(FunctionCall other);
  virtual ~FunctionCall() { }
  FunctionCall *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct Quantifier : public Node {

  std::shared_ptr<VarDecl> var;
  std::shared_ptr<Expr> step;

  Quantifier() = delete;
  Quantifier(const std::string &name, std::shared_ptr<TypeExpr> type,
    const location &loc);
  Quantifier(const std::string &name, std::shared_ptr<Expr> from,
    std::shared_ptr<Expr> to, const location &loc_);
  Quantifier(const std::string &name, std::shared_ptr<Expr> from,
    std::shared_ptr<Expr> to, std::shared_ptr<Expr> step_,
    const location &loc_);
  Quantifier(const Quantifier &other);
  Quantifier &operator=(Quantifier other);
  friend void swap(Quantifier &x, Quantifier &y) noexcept;
  virtual ~Quantifier() { }
  Quantifier *clone() const final;
  bool operator==(const Node &other) const final;

  void generate_header(std::ostream &out) const;
  void generate_footer(std::ostream &out) const;

 private:
  /* This constructor is delegated to internally.
   * HACK: This takes the arguments in a different order to avoid internal
   * constructor references being ambiguous.
   */
  Quantifier(const location &loc_, const std::string &name,
    std::shared_ptr<Expr> from, std::shared_ptr<Expr> to,
    std::shared_ptr<Expr> step_);

};

struct Exists : public Expr {

  std::shared_ptr<Quantifier> quantifier;
  std::shared_ptr<Expr> expr;

  Exists() = delete;
  Exists(std::shared_ptr<Quantifier> quantifier_, std::shared_ptr<Expr> expr_,
    const location &loc_);
  Exists(const Exists &other);
  Exists &operator=(Exists other);
  friend void swap(Exists &x, Exists &y) noexcept;
  virtual ~Exists() { }
  Exists *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct Forall : public Expr {

  std::shared_ptr<Quantifier> quantifier;
  std::shared_ptr<Expr> expr;

  Forall() = delete;
  Forall(std::shared_ptr<Quantifier> quantifier_, std::shared_ptr<Expr> expr_,
    const location &loc_);
  Forall(const Forall &other);
  Forall &operator=(Forall other);
  friend void swap(Forall &x, Forall &y) noexcept;
  virtual ~Forall() { }
  Forall *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  mpz_class constant_fold() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

}
