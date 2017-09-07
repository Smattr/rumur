#pragma once

#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Node.h>

namespace rumur {

class Stmt : public Node {

  public:
    using Node::Node;

    virtual void generate_stmt(std::ostream &out) const = 0;

};

class Assignment : public Stmt {

  public:
    std::shared_ptr<Lvalue> lhs;
    std::shared_ptr<Expr> rhs;

    explicit Assignment(std::shared_ptr<Lvalue> lhs, std::shared_ptr<Expr> rhs,
      const location &loc, Indexer &indexer);

    void validate() const final;

    void generate_stmt(std::ostream &out) const final;

};

}
