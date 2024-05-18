#pragma once

#include "location.hh"
#include <climits>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <string>
#include <utility>
#include <vector>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

// Forward declare to avoid circular #include
struct TypeDecl;
struct VarDecl;

struct RUMUR_API_WITH_RTTI TypeExpr : public Node {

  TypeExpr(const location &loc_);
  virtual ~TypeExpr() = default;

  // Whether this type is a primitive integer-like type.
  virtual bool is_simple() const;

  TypeExpr *clone() const override = 0;
  virtual mpz_class width() const;
  virtual mpz_class count() const = 0;
  virtual Ptr<TypeExpr> resolve() const;

  /* Numeric bounds of this type as valid C code. These are only valid to use on
   * TypeExprs for which is_simple() returns true.
   */
  virtual std::string lower_bound() const;
  virtual std::string upper_bound() const;

  // Get a string representation of this type
  virtual std::string to_string() const = 0;

  /* Whether this type's bounds are constant. Only valid for TypeExprs for which
   * is_simple() returns true.
   */
  virtual bool constant() const;

  // can a value of this type can be assigned to or compared with a value of the
  // given type?
  bool coerces_to(const TypeExpr &other) const;

  // Is this the type Boolean? Note that this only returns true for the actual
  // type Boolean, and not for TypeExprIDs that point at Boolean.
  virtual bool is_boolean() const;

protected:
  TypeExpr(const TypeExpr &) = default;
  TypeExpr &operator=(const TypeExpr &) = default;
};

struct RUMUR_API_WITH_RTTI Range : public TypeExpr {

  Ptr<Expr> min;
  Ptr<Expr> max;

  Range(const Ptr<Expr> &min_, const Ptr<Expr> &max_, const location &loc_);
  Range *clone() const override;
  virtual ~Range() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class count() const override;
  bool is_simple() const override;
  void validate() const override;

  std::string lower_bound() const override;
  std::string upper_bound() const override;
  std::string to_string() const override;
  bool constant() const override;
};

struct RUMUR_API_WITH_RTTI Scalarset : public TypeExpr {

  Ptr<Expr> bound;

  Scalarset(const Ptr<Expr> &bound_, const location &loc_);
  Scalarset *clone() const override;
  virtual ~Scalarset() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class count() const override;
  bool is_simple() const override;
  void validate() const override;

  std::string lower_bound() const override;
  std::string upper_bound() const override;
  std::string to_string() const override;
  bool constant() const override;
};

struct RUMUR_API_WITH_RTTI Enum : public TypeExpr {

  std::vector<std::pair<std::string, location>> members;

  // The range [unique_id, unique_id_limit) is usable by this node as
  // identifiers. Enum types need this specialisation due to the way references
  // to their members are resolved (see resolve_symbols()).
  size_t unique_id_limit = SIZE_MAX;

  Enum(const std::vector<std::pair<std::string, location>> &members_,
       const location &loc_);
  Enum *clone() const override;
  virtual ~Enum() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class count() const override;
  bool is_simple() const override;
  void validate() const override;

  std::string lower_bound() const override;
  std::string upper_bound() const override;
  std::string to_string() const override;
  bool constant() const override;
  bool is_boolean() const override;
};

struct RUMUR_API_WITH_RTTI Record : public TypeExpr {

  std::vector<Ptr<VarDecl>> fields;

  Record(const std::vector<Ptr<VarDecl>> &fields_, const location &loc_);
  Record *clone() const override;
  virtual ~Record() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class width() const override;
  mpz_class count() const override;
  std::string to_string() const override;
};

struct RUMUR_API_WITH_RTTI Array : public TypeExpr {

  Ptr<TypeExpr> index_type;
  Ptr<TypeExpr> element_type;

  Array(const Ptr<TypeExpr> &index_type_, const Ptr<TypeExpr> &element_type_,
        const location &loc_);
  Array *clone() const override;
  virtual ~Array() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class width() const override;
  mpz_class count() const override;
  void validate() const override;
  std::string to_string() const override;
};

struct RUMUR_API_WITH_RTTI TypeExprID : public TypeExpr {

  std::string name;
  Ptr<TypeDecl> referent;

  TypeExprID(const std::string &name_, const Ptr<TypeDecl> &referent_,
             const location &loc_);
  TypeExprID *clone() const override;
  virtual ~TypeExprID() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  mpz_class width() const override;
  mpz_class count() const override;
  bool is_simple() const override;
  Ptr<TypeExpr> resolve() const override;
  void validate() const override;

  std::string lower_bound() const override;
  std::string upper_bound() const override;
  std::string to_string() const override;
  bool constant() const override;
};

} // namespace rumur
