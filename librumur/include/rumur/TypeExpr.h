#pragma once

#include <climits>
#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// Forward declare to avoid circular #include
struct TypeDecl;
struct VarDecl;

struct TypeExpr : public Node {

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
};

struct Range : public TypeExpr {

  Ptr<Expr> min;
  Ptr<Expr> max;

  Range(const Ptr<Expr> &min_, const Ptr<Expr> &max_, const location &loc_);
  Range *clone() const final;
  virtual ~Range() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class count() const final;
  bool is_simple() const final;
  void validate() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
  std::string to_string() const final;
  bool constant() const final;
};

struct Scalarset : public TypeExpr {

  Ptr<Expr> bound;

  Scalarset(const Ptr<Expr> &bound_, const location &loc_);
  Scalarset *clone() const final;
  virtual ~Scalarset() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class count() const final;
  bool is_simple() const final;
  void validate() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
  std::string to_string() const final;
  bool constant() const final;
};

struct Enum : public TypeExpr {

  std::vector<std::pair<std::string, location>> members;

  // The range [unique_id, unique_id_limit) is usable by this node as
  // identifiers. Enum types need this specialisation due to the way references
  // to their members are resolved (see resolve_symbols()).
  size_t unique_id_limit = SIZE_MAX;

  Enum(const std::vector<std::pair<std::string, location>> &members_,
    const location &loc_);
  Enum *clone() const final;
  virtual ~Enum() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class count() const final;
  bool is_simple() const final;
  void validate() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
  std::string to_string() const final;
  bool constant() const final;
  bool is_boolean() const final;
};

struct Record : public TypeExpr {

  std::vector<Ptr<VarDecl>> fields;

  Record(const std::vector<Ptr<VarDecl>> &fields_, const location &loc_);
  Record *clone() const final;
  virtual ~Record() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class width() const final;
  mpz_class count() const final;
  std::string to_string() const final;
};

struct Array : public TypeExpr {

  Ptr<TypeExpr> index_type;
  Ptr<TypeExpr> element_type;

  Array(const Ptr<TypeExpr> &index_type_, const Ptr<TypeExpr> &element_type_,
    const location &loc_);
  Array *clone() const final;
  virtual ~Array() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class width() const final;
  mpz_class count() const final;
  void validate() const final;
  std::string to_string() const final;
};

struct TypeExprID : public TypeExpr {

  std::string name;
  Ptr<TypeDecl> referent;

  TypeExprID(const std::string &name_, const Ptr<TypeDecl> &referent_,
    const location &loc_);
  TypeExprID *clone() const final;
  virtual ~TypeExprID() = default;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  mpz_class width() const final;
  mpz_class count() const final;
  bool is_simple() const final;
  Ptr<TypeExpr> resolve() const final;
  void validate() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
  std::string to_string() const final;
  bool constant() const final;
};

}
