#pragma once

#include <cstdint>
#include <iostream>
#include "location.hh"
#include <memory>
#include <optional>
#include <rumur/Indexer.h>
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

    /* Emit some C++ code that implements an rvalue reference of this
     * expression. Implementers can assume the variable 's' is in scope that is
     * a pointer to the state. Expressions should be emitted inside brackets if
     * there is any possibility of an order-of-operations confusion when
     * embedding them in something else.
     */
    virtual void generate_read(std::ostream &out) const = 0;

    /* Emit some C++ code that implements an lvalue reference of this
     * expression. Note that it makes no sense for some expressions to be
     * lvalues (e.g. addition). Callers are expected to understand this and only
     * call this method on expressions for which it makes sense.
     */
    virtual void lvalue(std::ostream &out) const;

    virtual ~Expr() = 0;

};

class Ternary : public Expr {

  public:
    std::shared_ptr<Expr> cond;
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;

    explicit Ternary(std::shared_ptr<Expr> cond, std::shared_ptr<Expr> lhs,
      std::shared_ptr<Expr> rhs, const location &loc, Indexer &indexer) noexcept;

    void validate() const final;
    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

    /* Note that we do not override ``lvalue``. In Murphi, unlike C++, a ternary
     * expression cannot be an lvalue.
     */

};

class BinaryExpr : public Expr {

  public:
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;

    explicit BinaryExpr(std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs,
      const location &loc, Indexer &indexer) noexcept;

    void validate() const override;
    bool constant() const noexcept final;

};

class Implication : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Or : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class And : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class UnaryExpr : public Expr {

  public:

    std::shared_ptr<Expr> rhs;

    explicit UnaryExpr(std::shared_ptr<Expr> rhs, const location &loc,
      Indexer &indexer) noexcept;

    void validate() const override;
    bool constant() const noexcept final;

};

class Not : public UnaryExpr {

  public:
    using UnaryExpr::UnaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Lt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Leq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Gt : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Geq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Eq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Neq : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Add : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Sub : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Negative : public UnaryExpr {

  public:
    using UnaryExpr::UnaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Mul : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Div : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Mod : public BinaryExpr {

  public:
    using BinaryExpr::BinaryExpr;

    void validate() const final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

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
      const TypeExpr *type_of, const location &loc, Indexer &indexer);

    /* FIXME: This object is basically a proxy for other expression types, which
     * results in us re-implementing any applicable method here. It would be
     * simpler if we could just redirect these in a less verbose way.
     */
    void validate() const final;
    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;
    void lvalue(std::ostream &out) const final;

};

// FIXME: why do we need this class? Could this just be covered by ExprID?
class Var : public Expr {

  public:
    std::shared_ptr<VarDecl> decl;

    explicit Var(std::shared_ptr<VarDecl> decl, const location &loc,
      Indexer &indexer);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;
    void lvalue(std::ostream &out) const final;

};

class Field : public Expr {

  public:
    std::shared_ptr<Expr> record;
    std::string field;

    explicit Field(std::shared_ptr<Expr> record, const std::string &field,
      const location &loc, Indexer &indexer);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;
    void lvalue(std::ostream &out) const final;

};

class Element : public Expr {

  public:
    std::shared_ptr<Expr> array;
    std::shared_ptr<Expr> index;

    explicit Element(std::shared_ptr<Expr> array, std::shared_ptr<Expr> index,
      const location &loc, Indexer &indexer);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;
    void lvalue(std::ostream &out) const final;

};

class Quantifier : public Node {

  public:
    std::shared_ptr<VarDecl> var;
    std::optional<std::shared_ptr<Expr>> step;

    explicit Quantifier(const std::string &name, std::shared_ptr<TypeExpr> type,
      const location &loc, Indexer &indexer);
    explicit Quantifier(const std::string &name, std::shared_ptr<Expr> from,
      std::shared_ptr<Expr> to, const location &loc, Indexer &indexer);
    explicit Quantifier(const std::string &name, std::shared_ptr<Expr> from,
      std::shared_ptr<Expr> to, std::shared_ptr<Expr> step,
      const location &loc, Indexer &indexer);

  private:
    /* This constructor is delegated to internally.
     * HACK: This takes the arguments in a different order to avoid internal
     * constructor references being ambiguous.
     */
    explicit Quantifier(const location &loc, const std::string &name,
      std::shared_ptr<Expr> from, std::shared_ptr<Expr> to,
      std::optional<std::shared_ptr<Expr>> step, Indexer &indexer);

};

class Forall : public Expr {

  public:
    std::shared_ptr<Quantifier> quantifier;
    std::shared_ptr<Expr> expr;

    explicit Forall(std::shared_ptr<Quantifier> quantifier,
      std::shared_ptr<Expr> expr, const location &loc, Indexer &indexer);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

class Exists : public Expr {

  public:
    std::shared_ptr<Quantifier> quantifier;
    std::shared_ptr<Expr> expr;

    explicit Exists(std::shared_ptr<Quantifier> quantifier,
      std::shared_ptr<Expr> expr, const location &loc, Indexer &indexer);

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void generate_read(std::ostream &out) const final;

};

}
