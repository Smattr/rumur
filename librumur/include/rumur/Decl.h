#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <limits>
#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/TypeExpr.h>
#include <string>

namespace rumur {

struct Decl : public Node {

  std::string name;

  Decl() = delete;
  Decl(const std::string &name_, const location &loc_);
  Decl(const Decl&) = default;
  Decl(Decl&&) = default;
  Decl &operator=(const Decl&) = default;
  Decl &operator=(Decl&&) = default;
  virtual ~Decl() = 0;

  Decl *clone() const override = 0;
};

struct ExprDecl : public Decl {

  using Decl::Decl;
  ExprDecl() = delete;

  virtual ~ExprDecl() { }

  // Return true if this declaration is usable as an lvalue
  virtual bool is_lvalue() const = 0;

  virtual const TypeExpr *get_type() const = 0;
};

struct AliasDecl : public ExprDecl {

  std::shared_ptr<Expr> value;

  AliasDecl() = delete;
  AliasDecl(const std::string &name_, std::shared_ptr<Expr> value_,
    const location &loc_);
  AliasDecl(const AliasDecl &other);
  AliasDecl &operator=(AliasDecl other);
  friend void swap(AliasDecl &x, AliasDecl &y) noexcept;
  AliasDecl *clone() const final;
  virtual ~AliasDecl() { }

  bool operator==(const Node &other) const final;
  bool is_lvalue() const final;
  const TypeExpr *get_type() const final;
};

struct ConstDecl : public ExprDecl {

  std::shared_ptr<Expr> value;

  /* The type of this constant. Typically this will be NULL (untyped), but in
   * the case of enum members it will have the enum declaration as its type.
   */
  std::shared_ptr<TypeExpr> type;

  ConstDecl() = delete;
  ConstDecl(const std::string &name_, std::shared_ptr<Expr> value_,
    const location &loc_);
  ConstDecl(const std::string &name_, std::shared_ptr<Expr> value_,
    std::shared_ptr<TypeExpr> type_, const location &loc_);
  ConstDecl(const ConstDecl &other);
  ConstDecl &operator=(ConstDecl other);
  friend void swap(ConstDecl &x, ConstDecl &y) noexcept;
  ConstDecl *clone() const final;
  virtual ~ConstDecl() { }

  bool operator==(const Node &other) const final;
  bool is_lvalue() const final;
  void validate() const final;
  const TypeExpr *get_type() const final;
};

struct TypeDecl : public Decl {

  std::shared_ptr<TypeExpr> value;

  TypeDecl() = delete;
  TypeDecl(const std::string &name, std::shared_ptr<TypeExpr> value_,
    const location &loc);
  TypeDecl(const TypeDecl &other);
  TypeDecl &operator=(TypeDecl other);
  friend void swap(TypeDecl &x, TypeDecl &y) noexcept;
  TypeDecl *clone() const final;
  virtual ~TypeDecl() { }

  bool operator==(const Node &other) const final;
};

struct VarDecl : public ExprDecl {

  std::shared_ptr<TypeExpr> type;

  /* Offset within the model state. This is only relevant if this is a state
   * variable. We initially set it to an invalid value and rely on Model::reindex
   * setting this correctly later.
   */
  mpz_class offset = -1;

  /* Whether this variable is a read-only reference. E.g. a non-var parameter to
   * a function or procedure.
   */
  bool readonly = false;

  VarDecl() = delete;
  VarDecl(const std::string &name_, std::shared_ptr<TypeExpr> type_,
    const location &loc_);
  VarDecl(const VarDecl &other);
  VarDecl &operator=(VarDecl other);
  friend void swap(VarDecl &x, VarDecl &y) noexcept;
  VarDecl *clone() const final;
  virtual ~VarDecl() { }

  mpz_class count() const;
  mpz_class width() const;

  bool operator==(const Node &other) const final;
  bool is_lvalue() const final;
  const TypeExpr *get_type() const final;
};

}
