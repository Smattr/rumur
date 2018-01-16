#pragma once

#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// Forward declare to avoid circular #include
class VarDecl;

class TypeExpr : public Node {

  public:
    using Node::Node;
    TypeExpr() = delete;
    TypeExpr(const TypeExpr&) = default;
    TypeExpr(TypeExpr&&) = default;
    TypeExpr &operator=(const TypeExpr&) = default;
    TypeExpr &operator=(TypeExpr&&) = default;
    virtual ~TypeExpr() { }

    // Whether this type is a primitive integer-like type.
    virtual bool is_simple() const;

    TypeExpr *clone() const override = 0;

};

class SimpleTypeExpr : public TypeExpr {

  public:
    unsigned long index;

    SimpleTypeExpr() = delete;
    SimpleTypeExpr(const location &loc, Indexer &indexer);
    SimpleTypeExpr(const SimpleTypeExpr&) = default;
    SimpleTypeExpr(SimpleTypeExpr&&) = default;
    SimpleTypeExpr &operator=(const SimpleTypeExpr&) = default;
    SimpleTypeExpr &operator=(SimpleTypeExpr&&) = default;
    virtual ~SimpleTypeExpr() { }

    bool is_simple() const final;

    SimpleTypeExpr *clone() const override = 0;

};

class Range : public SimpleTypeExpr {

  public:
    Expr *min;
    Expr *max;

    Range() = delete;
    Range(Expr *min, Expr *max, const location &loc, Indexer &indexer);
    Range(const Range &other);
    Range &operator=(Range other);
    friend void swap(Range &x, Range &y) noexcept;
    Range *clone() const final;
    virtual ~Range();

    void validate() const final;
    void generate(std::ostream &out) const final;

};

class Enum : public SimpleTypeExpr {

  public:
    std::vector<ExprID> members;

    Enum() = delete;
    Enum(const std::vector<std::pair<std::string, location>> &members,
      const location &loc, Indexer &indexer);
    Enum(const Enum&) = default;
    Enum(Enum&&) = default;
    Enum &operator=(const Enum&) = default;
    Enum &operator=(Enum&&) = default;
    Enum *clone() const final;
    virtual ~Enum() { }

    void generate(std::ostream &out) const final;

};

class Record : public TypeExpr {

  public:
    std::vector<VarDecl*> fields;
    std::string name; // TODO: set this somewhere
    unsigned long index;

    Record() = delete;
    Record(std::vector<VarDecl*> &&fields, const location &loc, Indexer &indexer);
    Record(const Record &other);
    Record &operator=(Record other);
    friend void swap(Record &x, Record &y) noexcept;
    Record *clone() const final;
    virtual ~Record();

    void generate(std::ostream &out) const final;

};

class Array : public TypeExpr {

  public:
    TypeExpr *index_type;
    TypeExpr *element_type;
    std::string name; // TODO: set this somewhere
    unsigned long index;

    Array() = delete;
    Array(TypeExpr *index_type_, TypeExpr *element_type_, const location &loc_,
      Indexer &indexer);
    Array(const Array &other);
    Array &operator=(Array other);
    friend void swap(Array &x, Array &y) noexcept;
    Array *clone() const final;
    virtual ~Array();

    void generate(std::ostream &out) const final;

};

}
