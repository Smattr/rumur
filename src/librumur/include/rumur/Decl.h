#pragma once

#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/TypeExpr.h>
#include <string>

namespace rumur {

class Decl : public Node {

  public:

    std::string name;

    explicit Decl(const std::string &name, const location &loc);

    virtual ~Decl() {};

};

class ConstDecl : public Decl {

  public:

    std::shared_ptr<Expr> value;

    explicit ConstDecl(const std::string &name, std::shared_ptr<Expr> value, const location &loc);

    void validate() const final;

};

class TypeDecl : public Decl {

  public:
    std::shared_ptr<TypeExpr> value;

    explicit TypeDecl(const std::string &name, std::shared_ptr<TypeExpr> value, const location &loc);

    void validate() const final;

};

class VarDecl : public Decl {

  public:
    std::shared_ptr<TypeExpr> type;

    explicit VarDecl(const std::string &name, std::shared_ptr<TypeExpr> type, const location &loc);

};

}
