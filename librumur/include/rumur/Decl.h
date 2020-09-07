#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <limits>
#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <string>

namespace rumur {

struct Decl : public Node {

  std::string name;

  Decl(const std::string &name_, const location &loc_);
  virtual ~Decl() = 0;

  Decl *clone() const override = 0;
};

struct ExprDecl : public Decl {

  ExprDecl(const std::string &name_, const location &loc_);
  virtual ~ExprDecl() = default;

  // Return true if this declaration is usable as an lvalue
  virtual bool is_lvalue() const = 0;

  /* Return true if this declaration is to a resource cannot be modified. Note
   * that this is only relevant for declarations for which is_lvalue() returns
   * true. For non-lvalue declarations, this is always true.
   */
  virtual bool is_readonly() const = 0;

  virtual Ptr<TypeExpr> get_type() const = 0;

  ExprDecl *clone() const override = 0;
};

struct AliasDecl : public ExprDecl {

  Ptr<Expr> value;

  AliasDecl(const std::string &name_, const Ptr<Expr> &value_,
    const location &loc_);
  AliasDecl *clone() const final;
  virtual ~AliasDecl() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool is_lvalue() const final;
  bool is_readonly() const final;
  Ptr<TypeExpr> get_type() const final;
};

struct ConstDecl : public ExprDecl {

  Ptr<Expr> value;

  /* The type of this constant. Typically this will be NULL (untyped), but in
   * the case of enum members it will have the enum declaration as its type.
   */
  Ptr<TypeExpr> type;

  ConstDecl(const std::string &name_, const Ptr<Expr> &value_,
    const location &loc_);
  ConstDecl(const std::string &name_, const Ptr<Expr> &value_,
    const Ptr<TypeExpr> &type_, const location &loc_);
  ConstDecl *clone() const final;
  virtual ~ConstDecl() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  bool is_lvalue() const final;
  bool is_readonly() const final;
  void validate() const final;
  Ptr<TypeExpr> get_type() const final;
};

struct TypeDecl : public Decl {

  Ptr<TypeExpr> value;

  TypeDecl(const std::string &name, const Ptr<TypeExpr> &value_,
    const location &loc);
  TypeDecl *clone() const final;
  virtual ~TypeDecl() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;
};

struct VarDecl : public ExprDecl {

  Ptr<TypeExpr> type;

  /* Offset within the model state. This is only relevant if this is a state
   * variable. We initially set it to an invalid value and rely on Model::reindex
   * setting this correctly later.
   */
  mpz_class offset = -1;

  /* Whether this variable is a read-only reference. E.g. a non-var parameter to
   * a function or procedure.
   */
  bool readonly = false;

  VarDecl(const std::string &name_, const Ptr<TypeExpr> &type_,
    const location &loc_);
  VarDecl *clone() const final;
  virtual ~VarDecl() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class count() const;
  mpz_class width() const;

  /* Whether this variable declaration is a global; part of the state. That is,
   * not a local declaration, function parameter, etc.
   */
  bool is_in_state() const;

  bool is_lvalue() const final;
  bool is_readonly() const final;
  Ptr<TypeExpr> get_type() const final;
};

}
