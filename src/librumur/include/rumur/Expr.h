#pragma once

#include <cstdint>
#include "location.hh"
#include <memory>
#include <optional>
#include <rumur/Node.h>
#include <string>

namespace rumur {

// Forward declarations to avoid a circular #include
class TypeExpr;
class VarDecl;

class Expr : public Node {

  public:
    using Node::Node;

    // Whether an expression is a compile-time constant
    virtual bool constant() const noexcept = 0;

    /* The type of this expression. A nullptr indicates the type is equivalent
     * to a numeric literal; that is, an unbounded range.
     */
    virtual const TypeExpr *type() const noexcept = 0;

    /* Whether this expression can participate in arithmetic expressions (e.g.
     * addition). This is only true of literals and values of range types. That
     * is, booleans, enums and complex types are not arithmetic.
     */
    bool is_arithmetic() const noexcept;

    // If this expression is of boolean type.
    bool is_boolean() const noexcept;

    virtual ~Expr() = 0;

};

class Ternary : public Expr {

  public:
    std::shared_ptr<Expr> cond;
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;

    explicit Ternary(std::shared_ptr<Expr> cond, std::shared_ptr<Expr> lhs,
      std::shared_ptr<Expr> rhs, const location &loc) noexcept;

    void validate() const final;
    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;

};

class BinaryExpr : public Expr {

  public:
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;

    explicit BinaryExpr(std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs,
      const location &loc) noexcept;

    void validate() const override;
    bool constant() const noexcept final;

};

class Implication : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Or : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class And : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class UnaryExpr : public Expr {

  public:

    std::shared_ptr<Expr> rhs;

    explicit UnaryExpr(std::shared_ptr<Expr> rhs, const location &loc) noexcept;

    void validate() const override;
    bool constant() const noexcept final;

};

class Not : public UnaryExpr {

  public:
    using UnaryExpr::UnaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Lt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Leq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Gt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Geq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Eq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Neq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Add : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Sub : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Negative : public UnaryExpr {

  public:
    using UnaryExpr::UnaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Mul : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Div : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class Mod : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;

};

class ExprID : public Expr {

  public:
    std::string id;
    std::shared_ptr<Expr> value;

    /* We use a raw pointer here because we don't own this object and using a
     * std::shared_ptr would introduce a GC cycle. This is because the members
     * of an Enum (ExprIDs) have a type_of that points back to the Enum itself.
     * We set these in Enum's constructor so we can't easily use a std::weak_ptr
     * either.
     */
    const TypeExpr *type_of;

    explicit ExprID(const std::string &id, std::shared_ptr<Expr> value,
      const TypeExpr *type_of, const location &loc);

    void validate() const final;
    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;

};

class Var : public Expr {

  public:
    std::shared_ptr<VarDecl> decl;

    explicit Var(std::shared_ptr<VarDecl> decl, const location &loc);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;

};

class Field : public Expr {

  public:
    std::shared_ptr<Expr> record;
    std::string field;

    explicit Field(std::shared_ptr<Expr> record, const std::string &field,
      const location &loc);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;

};

class Element : public Expr {

  public:
    std::shared_ptr<Expr> array;
    std::shared_ptr<Expr> index;

    explicit Element(std::shared_ptr<Expr> array, std::shared_ptr<Expr> index,
      const location &loc);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;

};

class Quantifier : public Node {

  public:
    std::shared_ptr<VarDecl> var;
    std::optional<std::shared_ptr<Expr>> step;

    explicit Quantifier(const std::string &name, std::shared_ptr<TypeExpr> type,
      const location &loc);
    explicit Quantifier(const std::string &name, std::shared_ptr<Expr> from,
      std::shared_ptr<Expr> to, const location &loc);
    explicit Quantifier(const std::string &name, std::shared_ptr<Expr> from,
      std::shared_ptr<Expr> to, std::shared_ptr<Expr> step,
      const location &loc);

  private:
    /* This constructor is delegated to internally.
     * HACK: This takes the arguments in a different order to avoid internal
     * constructor references being ambiguous.
     */
    explicit Quantifier(const location &loc, const std::string &name,
      std::shared_ptr<Expr> from, std::shared_ptr<Expr> to,
      std::optional<std::shared_ptr<Expr>> step);

};

class Forall : public Expr {

  public:
    std::shared_ptr<Quantifier> quantifier;
    std::shared_ptr<Expr> expr;

    explicit Forall(std::shared_ptr<Quantifier> quantifier,
      std::shared_ptr<Expr> expr, const location &loc);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;

};

class Exists : public Expr {

  public:
    std::shared_ptr<Quantifier> quantifier;
    std::shared_ptr<Expr> expr;

    explicit Exists(std::shared_ptr<Quantifier> quantifier,
      std::shared_ptr<Expr> expr, const location &loc);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;

};

}
