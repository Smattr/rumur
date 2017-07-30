#pragma once

#include "Number.h"
#include <string>

namespace rumur {

class Decl {

  public:

    std::string name;

    Decl(const std::string &name);

    virtual ~Decl() {};

};

class ConstDecl : public Decl {

  public:

    Number *value;

    ConstDecl(const std::string &name, Number *value);

    ~ConstDecl();

};

class TypeDecl : public Decl {
};

class VarDecl : public Decl {
};

}
