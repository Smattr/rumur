#pragma once

#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Node.h>
#include <rumur/TypeExpr.h>
#include <string>

namespace rumur {

class Decl : public Node {

  public:

    std::string name;

    explicit Decl(const std::string &name, const location &loc);

    // Emit C++ code to define this entity.
    virtual void define(std::ostream &out) const = 0;

    virtual ~Decl() = 0;

};

class ConstDecl : public Decl {

  public:

    std::shared_ptr<Expr> value;

    explicit ConstDecl(const std::string &name, std::shared_ptr<Expr> value,
      const location &loc, Indexer &indexer);

    void validate() const final;
    void define(std::ostream &out) const final;

};

class TypeDecl : public Decl {

  public:
    std::shared_ptr<TypeExpr> value;

    explicit TypeDecl(const std::string &name, std::shared_ptr<TypeExpr> value,
      const location &loc, Indexer &indexer);

    void validate() const final;
    void define(std::ostream &out) const final;

};

class VarDecl : public Decl {

  public:
    std::shared_ptr<TypeExpr> type;
    bool local = false;

    explicit VarDecl(const std::string &name, std::shared_ptr<TypeExpr> type,
      const location &loc, Indexer &indexer);

    void define(std::ostream &out) const final;

};

}
