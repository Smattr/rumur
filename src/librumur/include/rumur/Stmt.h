#pragma once

#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>

namespace rumur {

class Stmt : public Node {

  public:
    using Node::Node;

};

class Assignment : public Stmt {

  public:
    Expr *lhs;
    Expr *rhs;

    explicit Assignment(Expr *lhs, Expr *rhs, const location &loc);

    virtual ~Assignment();

};

}
