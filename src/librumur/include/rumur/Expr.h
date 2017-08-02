#pragma once

namespace rumur {

class Expr {

  public:

    virtual ~Expr() = 0;

};

class Ternary : public Expr {

  public:

    Expr *cond;
    Expr *lhs;
    Expr *rhs;

    explicit Ternary(Expr *cond, Expr *lhs, Expr *rhs) noexcept;

    virtual ~Ternary();

};

class BinaryExpr : public Expr {

  public:

    Expr *lhs;
    Expr *rhs;

    explicit BinaryExpr(Expr *lhs, Expr *rhs) noexcept;

    virtual ~BinaryExpr() = 0;

};

class Implication : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Or : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class And : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class UnaryExpr : public Expr {

  public:

    Expr *rhs;

    explicit UnaryExpr(Expr *rhs) noexcept;

    virtual ~UnaryExpr() = 0;

};

class Not : public UnaryExpr {

  public:
    using UnaryExpr::UnaryExpr;

};

class Lt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Leq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Gt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Geq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Eq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Neq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Add : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Sub : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Negative : public UnaryExpr {

  public:
    using UnaryExpr::UnaryExpr;

};

class Mul : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Div : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

class Mod : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

};

}
