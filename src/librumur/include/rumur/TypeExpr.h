#pragma once

#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>

namespace rumur {

class TypeExpr : public Node {

  public:
    using Node::Node;

};

class Range : public TypeExpr {

  public:
    Expr *min;
    Expr *max;

    explicit Range(Expr *min, Expr *max, const location &loc);

    virtual ~Range();

};

}
