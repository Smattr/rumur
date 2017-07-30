#pragma once

#include "Number.h"
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

    Number *value;

    explicit ConstDecl(const std::string &name, Number *value);

    ~ConstDecl();

};

class TypeDecl : public Decl {
};

class VarDecl : public Decl {
};

}
