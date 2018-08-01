#pragma once

#include <cstdint>
#include <iostream>
#include "location.hh"
#include <rumur/Node.h>
#include <string>

namespace rumur {

// Forward declarations to avoid a circular #include
struct Decl;
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

  virtual int64_t constant_fold() const = 0;

  // Write out some C code that implements this expression.
  virtual void generate_rvalue(std::ostream &out) const = 0;

};

static inline std::ostream &operator<<(std::ostream &out, const Expr &e) {
  e.generate_rvalue(out);
  return out;
}

struct Ternary : public Expr {

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
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct BinaryExpr : public Expr {

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

struct BooleanBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  BooleanBinaryExpr() = delete;
  BooleanBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_);
};

struct Implication : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  Implication() = delete;
  Implication &operator=(Implication other);
  Implication *clone() const final;
  virtual ~Implication() { }

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Or : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  Or() = delete;
  Or &operator=(Or other);
  virtual ~Or() { }
  Or *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct And : public BooleanBinaryExpr {

  using BooleanBinaryExpr::BooleanBinaryExpr;
  And() = delete;
  And &operator=(And other);
  virtual ~And() { }
  And *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct UnaryExpr : public Expr {

  Expr *rhs;

  UnaryExpr() = delete;
  UnaryExpr(Expr *rhs_, const location &loc_);
  UnaryExpr(const UnaryExpr &other);
  friend void swap(UnaryExpr &x, UnaryExpr &y) noexcept;
  UnaryExpr *clone() const override = 0;
  virtual ~UnaryExpr();

  bool constant() const final;
};

struct Not : public UnaryExpr {

  using UnaryExpr::UnaryExpr;
  Not() = delete;
  Not(Expr *rhs_, const location &loc_);
  Not &operator=(Not other);
  virtual ~Not() { }
  Not *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct ComparisonBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  ComparisonBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_);
  ComparisonBinaryExpr() = delete;
};

struct Lt : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Lt() = delete;
  Lt &operator=(Lt other);
  virtual ~Lt() { }
  Lt *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Leq : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Leq() = delete;
  Leq &operator=(Leq other);
  virtual ~Leq() { }
  Leq *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Gt : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Gt() = delete;
  Gt &operator=(Gt other);
  virtual ~Gt() { }
  Gt *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Geq : public ComparisonBinaryExpr {

  using ComparisonBinaryExpr::ComparisonBinaryExpr;
  Geq() = delete;
  Geq &operator=(Geq other);
  virtual ~Geq() { }
  Geq *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct EquatableBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  EquatableBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_);
  EquatableBinaryExpr() = delete;
};

struct Eq : public EquatableBinaryExpr {

  using EquatableBinaryExpr::EquatableBinaryExpr;
  Eq() = delete;
  Eq &operator=(Eq other);
  virtual ~Eq() { }
  Eq *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Neq : public EquatableBinaryExpr {

  using EquatableBinaryExpr::EquatableBinaryExpr;
  Neq() = delete;
  Neq &operator=(Neq other);
  virtual ~Neq() { }
  Neq *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct ArithmeticBinaryExpr : public BinaryExpr {

  using BinaryExpr::BinaryExpr;
  ArithmeticBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc);
  ArithmeticBinaryExpr() = delete;
};

struct Add : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Add() = delete;
  Add &operator=(Add other);
  virtual ~Add() { }
  Add *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Sub : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Sub() = delete;
  Sub &operator=(Sub other);
  virtual ~Sub() { }
  Sub *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Negative : public UnaryExpr {

  using UnaryExpr::UnaryExpr;
  Negative() = delete;
  Negative(Expr *rhs_, const location &loc_);
  Negative &operator=(Negative other);
  virtual ~Negative() { }
  Negative *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Mul : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Mul() = delete;
  Mul &operator=(Mul other);
  virtual ~Mul() { }
  Mul *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Div : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Div() = delete;
  Div &operator=(Div other);
  virtual ~Div() { }
  Div *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Mod : public ArithmeticBinaryExpr {

  using ArithmeticBinaryExpr::ArithmeticBinaryExpr;
  Mod() = delete;
  Mod &operator=(Mod other);
  virtual ~Mod() { }
  Mod *clone() const final;

  const TypeExpr *type() const final;
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Lvalue : public Expr {

  using Expr::Expr;
  Lvalue() = delete;
  Lvalue(const Lvalue&) = default;
  Lvalue(Lvalue&&) = default;
  Lvalue &operator=(const Lvalue&) = default;
  Lvalue &operator=(Lvalue&&) = default;
  void generate_rvalue(std::ostream &out) const final;
  void generate_lvalue(std::ostream &out) const;
  virtual void generate(std::ostream &out, bool lvalue) const = 0;
  virtual ~Lvalue() = 0;
  virtual Lvalue *clone() const = 0;

};

struct ExprID : public Lvalue {

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
  void generate(std::ostream &out, bool lvalue) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Field : public Lvalue {

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
  void generate(std::ostream &out, bool lvalue) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Element : public Lvalue {

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
  void generate(std::ostream &out, bool lvalue) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Quantifier : public Node {

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
  bool operator==(const Node &other) const final;

  void generate_header(std::ostream &out) const;
  void generate_footer(std::ostream &out) const;

 private:
  /* This constructor is delegated to internally.
   * HACK: This takes the arguments in a different order to avoid internal
   * constructor references being ambiguous.
   */
  Quantifier(const location &loc_, const std::string &name, Expr *from,
    Expr *to, Expr *step_);

};

struct Exists : public Expr {

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
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

struct Forall : public Expr {

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
  void generate_rvalue(std::ostream &out) const final;
  int64_t constant_fold() const final;
  bool operator==(const Node &other) const final;
};

}
