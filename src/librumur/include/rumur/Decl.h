#pragma once

#include <rumur/Expr.h>
#include <string>

namespace rumur {

class Decl {

  public:

    std::string name;

    explicit Decl(const std::string &name);

    virtual ~Decl() {};

};

class ConstDecl : public Decl {

  public:

    Expr *value;

    explicit ConstDecl(const std::string &name, Expr *value);

    ~ConstDecl();

};

class TypeDecl : public Decl {
};

class VarDecl : public Decl {
};

}
