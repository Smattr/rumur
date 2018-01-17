#pragma once

#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/TypeExpr.h>
#include <string>

namespace rumur {

class Decl : public Node {

  public:
    std::string name;

    Decl() = delete;
    Decl(const std::string &name, const location &loc);
    Decl(const Decl&) = default;
    Decl(Decl&&) = default;
    Decl &operator=(const Decl&) = default;
    Decl &operator=(Decl&&) = default;
    virtual ~Decl() = 0;

    Decl *clone() const override = 0;

};

class ConstDecl : public Decl {

  public:
    Expr *value;

    ConstDecl() = delete;
    ConstDecl(const std::string &name, const Expr *value,
      const location &loc);
    ConstDecl(const ConstDecl &other);
    ConstDecl &operator=(ConstDecl other);
    friend void swap(ConstDecl &x, ConstDecl &y) noexcept;
    ConstDecl *clone() const final;
    virtual ~ConstDecl();

    void validate() const final;
    void generate(std::ostream &out) const final;

};

class TypeDecl : public Decl {

  public:
    TypeExpr *value;

    TypeDecl() = delete;
    TypeDecl(const std::string &name, TypeExpr *value,
      const location &loc);
    TypeDecl(const TypeDecl &other);
    TypeDecl &operator=(TypeDecl other);
    friend void swap(TypeDecl &x, TypeDecl &y) noexcept;
    TypeDecl *clone() const final;
    virtual ~TypeDecl();

    void validate() const final;
    void generate(std::ostream &out) const final;

};

class VarDecl : public Decl {

  public:
    TypeExpr *type;
    bool local = false;

    VarDecl() = delete;
    VarDecl(const std::string &name, TypeExpr *type,
      const location &loc);
    VarDecl(const VarDecl &other);
    VarDecl &operator=(VarDecl other);
    friend void swap(VarDecl &x, VarDecl &y) noexcept;
    VarDecl *clone() const final;
    virtual ~VarDecl();

    void generate(std::ostream &out) const final;

};

}
