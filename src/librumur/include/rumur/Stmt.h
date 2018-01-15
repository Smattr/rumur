#pragma once

#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Node.h>

namespace rumur {

class Stmt : public Node {

  public:
    using Node::Node;

    Stmt() = delete;
    Stmt(const Stmt&) = default;
    Stmt(Stmt&&) = default;
    Stmt &operator=(const Stmt&) = default;
    Stmt &operator=(Stmt&&) = default;
    virtual ~Stmt() { }
    virtual Stmt *clone() const = 0;

};

class Assignment : public Stmt {

  public:
    Lvalue *lhs;
    Expr *rhs;

    Assignment() = delete;
    Assignment(Lvalue *lhs, Expr *rhs, const location &loc, Indexer &indexer);
    Assignment(const Assignment &other);
    Assignment &operator=(Assignment other);
    friend void swap(Assignment &x, Assignment &y) noexcept;
    Assignment *clone() const final;
    virtual ~Assignment();

    void validate() const final;

    void generate(std::ostream &out) const final;

};

}
