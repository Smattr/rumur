#pragma once

#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>

namespace rumur {

class Stmt : public Node {

  public:
    using Node::Node;

};

class Assignment : public Stmt {

  public:
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;

    explicit Assignment(std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs,
      const location &loc);

};

}
