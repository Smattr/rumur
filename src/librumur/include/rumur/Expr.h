#pragma once

#include "location.hh"
#include <rumur/Node.h>
#include <string>

namespace rumur {

class Expr : public Node {

  public:
    using Node::Node;

    // Whether an expression is a compile-time constant
    virtual bool constant() const noexcept = 0;

    virtual ~Expr() = 0;

};

class Ternary : public Expr {

  public:

    Expr *cond;
    Expr *lhs;
    Expr *rhs;

    explicit Ternary(Expr *cond, Expr *lhs, Expr *rhs, const location &loc)
        noexcept;

    bool constant() const noexcept final;

    virtual ~Ternary();

};

class BinaryExpr : public Expr {

  public:

    Expr *lhs;
    Expr *rhs;

    explicit BinaryExpr(Expr *lhs, Expr *rhs, const location &loc) noexcept;

    bool constant() const noexcept final;

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

    explicit UnaryExpr(Expr *rhs, const location &loc) noexcept;

    bool constant() const noexcept final;

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

class ExprID : public Expr {

  public:
    std::string id;
    Expr *value;

    explicit ExprID(const std::string &id, Expr *value, const location &loc);

    bool constant() const noexcept final;

};

}
