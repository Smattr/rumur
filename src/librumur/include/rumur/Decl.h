#pragma once

#include "location.hh"
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

    Expr *value;

    explicit ConstDecl(const std::string &name, Expr *value, const location &loc);

    ~ConstDecl();

};

class TypeDecl : public Decl {

  public:
    TypeExpr *value;

    explicit TypeDecl(const std::string &name, TypeExpr *value, const location &loc);

    ~TypeDecl();

};

class VarDecl : public Decl {

  public:
    std::string name;
    TypeExpr *type;

    explicit VarDecl(const std::string &name, TypeExpr *type, const location &loc);

    virtual ~VarDecl();

};

}
