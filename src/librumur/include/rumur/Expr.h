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

  /* Whether this expression can participate in arithmetic expressions (e.g.
   * addition). This is only true of literals and values of range types. That
   * is, booleans, enums and complex types are not arithmetic.
   */
  bool is_arithmetic() const;

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
  void validate() const final;
  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

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
  void validate() const override;
  bool constant() const final;

};

class Implication : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Implication() = delete;
  Implication &operator=(Implication other);
  Implication *clone() const final;
  virtual ~Implication() { }

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Or : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Or() = delete;
  Or &operator=(Or other);
  virtual ~Or() { }
  Or *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class And : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  And() = delete;
  And &operator=(And other);
  virtual ~And() { }
  And *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

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

  void validate() const override;
  bool constant() const final;

};

class Not : public UnaryExpr {

 public:
  using UnaryExpr::UnaryExpr;
  Not() = delete;
  Not &operator=(Not other);
  virtual ~Not() { }
  Not *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Lt : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Lt() = delete;
  Lt &operator=(Lt other);
  virtual ~Lt() { }
  Lt *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Leq : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Leq() = delete;
  Leq &operator=(Leq other);
  virtual ~Leq() { }
  Leq *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Gt : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Gt() = delete;
  Gt &operator=(Gt other);
  virtual ~Gt() { }
  Gt *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Geq : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Geq() = delete;
  Geq &operator=(Geq other);
  virtual ~Geq() { }
  Geq *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Eq : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Eq() = delete;
  Eq &operator=(Eq other);
  virtual ~Eq() { }
  Eq *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Neq : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Neq() = delete;
  Neq &operator=(Neq other);
  virtual ~Neq() { }
  Neq *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Add : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Add() = delete;
  Add &operator=(Add other);
  virtual ~Add() { }
  Add *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Sub : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Sub() = delete;
  Sub &operator=(Sub other);
  virtual ~Sub() { }
  Sub *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Negative : public UnaryExpr {

 public:
  using UnaryExpr::UnaryExpr;
  Negative() = delete;
  Negative &operator=(Negative other);
  virtual ~Negative() { }
  Negative *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Mul : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Mul() = delete;
  Mul &operator=(Mul other);
  virtual ~Mul() { }
  Mul *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Div : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Div() = delete;
  Div &operator=(Div other);
  virtual ~Div() { }
  Div *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

};

class Mod : public BinaryExpr {

 public:
  using BinaryExpr::BinaryExpr;
  Mod() = delete;
  Mod &operator=(Mod other);
  virtual ~Mod() { }
  Mod *clone() const final;

  void validate() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

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

  /* FIXME: This object is basically a proxy for other expression types, which
   * results in us re-implementing any applicable method here. It would be
   * simpler if we could just redirect these in a less verbose way.
   */
  void validate() const final;
  bool constant() const final;
  const TypeExpr *type() const final;
  void generate(std::ostream &out) const final;
  int64_t constant_fold() const final;

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

};

}
