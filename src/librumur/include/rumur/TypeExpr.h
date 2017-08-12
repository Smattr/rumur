#pragma once

#include "location.hh"
#include <rumur/Expr.h>
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

    explicit Enum(const std::vector<std::pair<std::string, location>> &members, const location &loc);

    virtual ~Enum();

};

class Record : public TypeExpr {

  public:
    std::vector<VarDecl*> fields;

    explicit Record(std::vector<VarDecl*> &&fields, const location &loc);

    virtual ~Record();

};

}
