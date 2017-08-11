#pragma once

#include <cstdint>
#include "location.hh"
#include <rumur/Node.h>
#include <string>

namespace rumur {

// Forward declarations to avoid a circular #include
class TypeExpr;

class Expr : public Node {

  public:
    using Node::Node;

    // Whether an expression is a compile-time constant
    virtual bool constant() const noexcept = 0;

    /* The type of this expression. A nullptr indicates the type is equivalent
     * to a numeric literal; that is, an unbounded range.
     */
    virtual const TypeExpr *type() const noexcept = 0;

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

    const TypeExpr *type() const noexcept final;

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

    const TypeExpr *type() const noexcept final;

};

class Or : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class And : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

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

    const TypeExpr *type() const noexcept final;

};

class Lt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Leq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Gt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Geq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Eq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Neq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Add : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Sub : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Negative : public UnaryExpr {

  public:
    using UnaryExpr::UnaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Mul : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Div : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class Mod : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    const TypeExpr *type() const noexcept final;

};

class ExprID : public Expr {

  public:
    std::string id;
    const Expr *value;
    const TypeExpr *type_of;

    explicit ExprID(const std::string &id, const Expr *value, const TypeExpr *type_of, const location &loc);

    bool constant() const noexcept final;

    const TypeExpr *type() const noexcept final;

    // Note that we don't delete `value` or `type_of` because we don't own them.

};

}
