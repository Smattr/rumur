#pragma once

#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <string>
#include <vector>

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

class TypeExprID : public TypeExpr {

  public:
    std::string id;
    const TypeExpr *value;

    explicit TypeExprID(const std::string &id, const TypeExpr *value, const location &loc);

};

class Enum : public TypeExpr {

  public:
    std::vector<ExprID*> members;
    std::vector<Number*> representations;

    explicit Enum(std::vector<ExprID*> &&members, const location &loc);

    virtual ~Enum();

};

}
