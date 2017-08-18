#pragma once

#include "location.hh"
#include <memory>
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
    std::shared_ptr<Expr> min;
    std::shared_ptr<Expr> max;

    explicit Range(std::shared_ptr<Expr> min, std::shared_ptr<Expr> max,
      const location &loc);

    void validate() const final;

};

class TypeExprID : public TypeExpr {

  public:
    std::string id;
    std::shared_ptr<TypeExpr> value;

    explicit TypeExprID(const std::string &id, std::shared_ptr<TypeExpr> value,
      const location &loc);

};

class Enum : public TypeExpr {

  public:
    std::vector<std::shared_ptr<ExprID>> members;

    explicit Enum(const std::vector<std::pair<std::string, location>> &members,
      const location &loc);

};

class Record : public TypeExpr {

  public:
    std::vector<std::shared_ptr<VarDecl>> fields;

    explicit Record(std::vector<std::shared_ptr<VarDecl>> &&fields,
      const location &loc);

};

}
