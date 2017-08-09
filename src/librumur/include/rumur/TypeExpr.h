#pragma once

#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>
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
    TypeExpr *value;

    explicit TypeExprID(const std::string &id, TypeExpr *value, const location &loc);

    virtual ~TypeExprID();

};

class Enum : public TypeExpr {

  public:
    std::vector<EnumValue*> members;

    explicit Enum(std::vector<EnumValue*> &&members, const location &loc);

    virtual ~Enum();

};

}
