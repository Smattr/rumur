#pragma once

#include <iostream>
#include "location.hh"
#include <memory>
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

    // Whether this type is a primitive integer-like type.
    virtual bool is_simple() const;

    /* Generate code for an rvalue of the minimum or maximum of this type. These
     * are only valid to call on a type for which is_simple returns true.
     */
    virtual void generate_min(std::ostream &out) const;
    virtual void generate_max(std::ostream &out) const;

    /* Get names for getters and setters dependent on this particular type. It
     * is an error to call any of these on an inconsistent type (e.g.
     * field_reader on a type that is not a Record).
     */
    virtual std::string field_reader(const std::string &field) const;
    virtual std::string field_writer(const std::string &field) const;
    virtual std::string element_reader() const;
    virtual std::string element_writer() const;

    // Emit C++ definitions relevant to this type.
    virtual void define(std::ostream &out) const = 0;

};

class SimpleTypeExpr : public TypeExpr {

  public:
    unsigned long index;

    explicit SimpleTypeExpr(const location &loc, Indexer &indexer);

    bool is_simple() const final;

    /* Emit a C++ function name for reading/writing this type, respectively. */
    void reader(std::ostream &out) const;
    void writer(std::ostream &out) const;

};

class Range : public SimpleTypeExpr {

  public:
    std::shared_ptr<Expr> min;
    std::shared_ptr<Expr> max;

    explicit Range(std::shared_ptr<Expr> min, std::shared_ptr<Expr> max,
      const location &loc, Indexer &indexer);

    void validate() const final;
    void generate_min(std::ostream &out) const final;
    void generate_max(std::ostream &out) const final;
    void define(std::ostream &out) const final;

};

class Enum : public SimpleTypeExpr {

  public:
    std::vector<std::shared_ptr<ExprID>> members;

    explicit Enum(const std::vector<std::pair<std::string, location>> &members,
      const location &loc, Indexer &indexer);

    void generate_min(std::ostream &out) const final;
    void generate_max(std::ostream &out) const final;
    void define(std::ostream &out) const final;

};

class Record : public TypeExpr {

  public:
    std::vector<std::shared_ptr<VarDecl>> fields;
    std::string name; // TODO: set this somewhere
    unsigned long index;

    explicit Record(std::vector<std::shared_ptr<VarDecl>> &&fields,
      const location &loc, Indexer &indexer);

    void field_referencer(std::ostream &out, const std::string &field) const;
    std::string field_reader(const std::string &field) const final;
    std::string field_writer(const std::string &field) const final;
    void define(std::ostream &out) const final;

};

class Array : public TypeExpr {

  public:
    std::shared_ptr<TypeExpr> index_type;
    std::shared_ptr<TypeExpr> element_type;
    std::string name; // TODO: set this somewhere
    unsigned long index;

    explicit Array(std::shared_ptr<TypeExpr> index_type_,
      std::shared_ptr<TypeExpr> element_type_, const location &loc_,
      Indexer &indexer);

    void element_referencer(std::ostream &out) const;
    std::string element_reader() const final;
    std::string element_writer() const final;
    void define(std::ostream &out) const final;

};

}
