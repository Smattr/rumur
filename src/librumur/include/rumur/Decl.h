#pragma once

#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>
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
};

class VarDecl : public Decl {
};

}
