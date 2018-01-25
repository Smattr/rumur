#pragma once

#include <cstdint>
#include <iostream>
#include "location.hh"
#include <rumur/Node.h>
#include <string>

namespace rumur {

// Forward declarations to avoid a circular #include
class Decl;
class TypeExpr;
class VarDecl;

class Expr : public Node {

 public:
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

  virtual int64_t constant_fold() const = 0;

};

class Ternary : public Expr {

 public:
  Expr *cond;
  Expr *lhs;
  Expr *rhs;

  Ternary() = delete;
  Ternary(Expr *cond_, Expr *lhs_, Expr *rhs_, const location &loc_);
  Ternary(const Ternary &other);
  Ternary(Ternary&&) = default;
  Ternary &operator=(Ternary other);
  friend void swap(Ternary &x, Ternary &y) noexcept;
  virtual ~Ternary();

  Ternary *clone() const final;
  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class BinaryExpr : public Expr {

 public:
  Expr *lhs;
  Expr *rhs;

  BinaryExpr() = delete;
  BinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_);
  BinaryExpr(const BinaryExpr &other);
  BinaryExpr &operator=(const BinaryExpr&) = delete;
  BinaryExpr &operator=(BinaryExpr&&) = delete;
  friend void swap(BinaryExpr &x, BinaryExpr &y) noexcept;
  virtual ~BinaryExpr();

  BinaryExpr *clone() const override = 0;
  bool constant() const final;
};

class BooleanBinaryExpr : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  BooleanBinaryExpr() = delete;
  BooleanBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_);
};

class Implication : public BooleanBinaryExpr {

 public:
  using BooleanBinaryExpr::BooleanBinaryExpr;
  Implication() = delete;
  Implication &operator=(Implication other);
  Implication *clone() const final;
  virtual ~Implication() { }

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Or : public BooleanBinaryExpr {

 public:
  using BooleanBinaryExpr::BooleanBinaryExpr;
  Or() = delete;
  Or &operator=(Or other);
  virtual ~Or() { }
  Or *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class And : public BooleanBinaryExpr {

 public:
  using BooleanBinaryExpr::BooleanBinaryExpr;
  And() = delete;
  And &operator=(And other);
  virtual ~And() { }
  And *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class UnaryExpr : public Expr {

 public:
  Expr *rhs;

  UnaryExpr() = delete;
  UnaryExpr(Expr *rhs_, const location &loc_);
  UnaryExpr(const UnaryExpr &other);
  friend void swap(UnaryExpr &x, UnaryExpr &y) noexcept;
  UnaryExpr *clone() const override = 0;
  virtual ~UnaryExpr();

  bool constant() const final;
};

class Not : public UnaryExpr {

 public:
  using UnaryExpr::UnaryExpr;
  Not() = delete;
  Not(Expr *rhs_, const location &loc_);
  Not &operator=(Not other);
  virtual ~Not() { }
  Not *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class ComparisonBinaryExpr : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  ComparisonBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_);
  ComparisonBinaryExpr() = delete;
};

class Lt : public ComparisonBinaryExpr {

 public:
  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Lt() = delete;
  Lt &operator=(Lt other);
  virtual ~Lt() { }
  Lt *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Leq : public ComparisonBinaryExpr {

 public:
  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Leq() = delete;
  Leq &operator=(Leq other);
  virtual ~Leq() { }
  Leq *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Gt : public ComparisonBinaryExpr {

 public:
  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Gt() = delete;
  Gt &operator=(Gt other);
  virtual ~Gt() { }
  Gt *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Geq : public ComparisonBinaryExpr {

 public:
  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Geq() = delete;
  Geq &operator=(Geq other);
  virtual ~Geq() { }
  Geq *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class EquatableBinaryExpr : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  EquatableBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_);
  EquatableBinaryExpr() = delete;
};

class Eq : public EquatableBinaryExpr {

 public:
  using EquatableBinaryExpr::EquatableBinaryExpr;
  Eq() = delete;
  Eq &operator=(Eq other);
  virtual ~Eq() { }
  Eq *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Neq : public EquatableBinaryExpr {

 public:
  using EquatableBinaryExpr::EquatableBinaryExpr;
  Neq() = delete;
  Neq &operator=(Neq other);
  virtual ~Neq() { }
  Neq *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class ArithmeticBinaryExpr : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  ArithmeticBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc);
  ArithmeticBinaryExpr() = delete;
};

class Add : public ArithmeticBinaryExpr {

 public:
  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Add() = delete;
  Add &operator=(Add other);
  virtual ~Add() { }
  Add *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Sub : public ArithmeticBinaryExpr {

 public:
  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Sub() = delete;
  Sub &operator=(Sub other);
  virtual ~Sub() { }
  Sub *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Negative : public UnaryExpr {

 public:
  using UnaryExpr::UnaryExpr;
  Negative() = delete;
  Negative(Expr *rhs_, const location &loc_);
  Negative &operator=(Negative other);
  virtual ~Negative() { }
  Negative *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Mul : public ArithmeticBinaryExpr {

 public:
  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Mul() = delete;
  Mul &operator=(Mul other);
  virtual ~Mul() { }
  Mul *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Div : public ArithmeticBinaryExpr {

 public:
  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Div() = delete;
  Div &operator=(Div other);
  virtual ~Div() { }
  Div *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Mod : public ArithmeticBinaryExpr {

 public:
  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Mod() = delete;
  Mod &operator=(Mod other);
  virtual ~Mod() { }
  Mod *clone() const final;

  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Lvalue : public Expr {

 public:
  using Expr::Expr;
  Lvalue() = delete;
  Lvalue(const Lvalue&) = default;
  Lvalue(Lvalue&&) = default;
  Lvalue &operator=(const Lvalue&) = default;
  Lvalue &operator=(Lvalue&&) = default;
  virtual ~Lvalue() = 0;
  virtual Lvalue *clone() const = 0;

};

class ExprID : public Lvalue {

 public:
  std::string id;
  Decl *value;

  ExprID() = delete;
  ExprID(const std::string &id_, const Decl *value_, const location &loc_);
  ExprID(const ExprID &other);
  ExprID &operator=(ExprID other);
  friend void swap(ExprID &x, ExprID &y) noexcept;
  virtual ~ExprID();
  ExprID *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Field : public Lvalue {

 public:
  Lvalue *record;
  std::string field;

  Field() = delete;
  Field(Lvalue *record_, const std::string &field_, const location &loc_);
  Field(const Field &other);
  friend void swap(Field &x, Field &y) noexcept;
  Field &operator=(Field other);
  virtual ~Field();
  Field *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Element : public Lvalue {

 public:
  Lvalue *array;
  Expr *index;

  Element() = delete;
  Element(Lvalue *array_, Expr *index_, const location &loc_);
  Element(const Element &other);
  friend void swap(Element &x, Element &y) noexcept;
  Element &operator=(Element other);
  virtual ~Element();
  Element *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Quantifier : public Node {

 public:
  VarDecl *var;
  Expr *step;

  Quantifier() = delete;
  Quantifier(const std::string &name, TypeExpr *type, const location &loc);
  Quantifier(const std::string &name, Expr *from, Expr *to,
      const location &loc_);
  Quantifier(const std::string &name, Expr *from, Expr *to, Expr *step_,
    const location &loc_);
  Quantifier(const Quantifier &other);
  Quantifier &operator=(Quantifier other);
  friend void swap(Quantifier &x, Quantifier &y) noexcept;
  virtual ~Quantifier();
  Quantifier *clone() const final;
  void generate(std::ostream &out) const final;
  bool operator==(const Node &other) const final;

 private:
  /* This constructor is delegated to internally.
   * HACK: This takes the arguments in a different order to avoid internal
   * constructor references being ambiguous.
   */
  Quantifier(const location &loc_, const std::string &name, Expr *from,
    Expr *to, Expr *step_);

};

class Exists : public Expr {

 public:
  Quantifier *quantifier;
  Expr *expr;

  Exists() = delete;
  Exists(Quantifier *quantifier_, Expr *expr_, const location &loc_);
  Exists(const Exists &other);
  Exists &operator=(Exists other);
  friend void swap(Exists &x, Exists &y) noexcept;
  virtual ~Exists();
  Exists *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

class Forall : public Expr {

 public:
  Quantifier *quantifier;
  Expr *expr;

  Forall() = delete;
  Forall(Quantifier *quantifier_, Expr *expr_, const location &loc_);
  Forall(const Forall &other);
  Forall &operator=(Forall other);
  friend void swap(Forall &x, Forall &y) noexcept;
  virtual ~Forall();
  Forall *clone() const final;

  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

}
