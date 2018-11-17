#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <rumur/Ptr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// Forward declare to avoid circular #include
struct VarDecl;

struct TypeExpr : public Node {

  using Node::Node;
  TypeExpr() = delete;
  TypeExpr(const TypeExpr&) = default;
  TypeExpr(TypeExpr&&) = default;
  TypeExpr &operator=(const TypeExpr&) = default;
  TypeExpr &operator=(TypeExpr&&) = default;
  virtual ~TypeExpr() = default;

  // Whether this type is a primitive integer-like type.
  virtual bool is_simple() const;

  TypeExpr *clone() const override = 0;
  virtual mpz_class width() const;
  virtual mpz_class count() const = 0;
  virtual const TypeExpr *resolve() const;

  /* Numeric bounds of this type as valid C code. These are only valid to use on
   * TypeExprs for which is_simple() returns true.
   */
  virtual std::string lower_bound() const;
  virtual std::string upper_bound() const;
};

struct Range : public TypeExpr {

  Ptr<Expr> min;
  Ptr<Expr> max;

  Range() = delete;
  Range(const Ptr<Expr> &min_, const Ptr<Expr> &max_, const location &loc_);
  Range(const Range &other);
  Range &operator=(Range other);
  friend void swap(Range &x, Range &y) noexcept;
  Range *clone() const final;
  virtual ~Range() = default;

  mpz_class count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;
  void validate() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
};

struct Scalarset : public TypeExpr {

  Ptr<Expr> bound;

  Scalarset() = delete;
  Scalarset(const Ptr<Expr> &bound_, const location &loc_);
  Scalarset(const Scalarset &other);
  Scalarset &operator=(Scalarset other);
  friend void swap(Scalarset &x, Scalarset &y) noexcept;
  Scalarset *clone() const final;
  virtual ~Scalarset() = default;

  mpz_class count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;
  void validate() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
};

struct Enum : public TypeExpr {

  std::vector<std::pair<std::string, location>> members;

  Enum() = delete;
  Enum(const std::vector<std::pair<std::string, location>> &&members_,
    const location &loc_);
  Enum(const Enum&) = default;
  Enum(Enum&&) = default;
  Enum &operator=(const Enum&) = default;
  Enum &operator=(Enum&&) = default;
  Enum *clone() const final;
  virtual ~Enum() = default;

  mpz_class count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
};

struct Record : public TypeExpr {

  std::vector<Ptr<VarDecl>> fields;

  Record() = delete;
  Record(const std::vector<Ptr<VarDecl>> &fields_, const location &loc_);
  Record(const Record &other);
  Record &operator=(Record other);
  friend void swap(Record &x, Record &y) noexcept;
  Record *clone() const final;
  virtual ~Record() = default;

  mpz_class width() const final;
  mpz_class count() const final;
  bool operator==(const Node &other) const final;
};

struct Array : public TypeExpr {

  std::shared_ptr<TypeExpr> index_type;
  std::shared_ptr<TypeExpr> element_type;

  Array() = delete;
  Array(std::shared_ptr<TypeExpr> index_type_,
    std::shared_ptr<TypeExpr> element_type_, const location &loc_);
  Array(const Array &other);
  Array &operator=(Array other);
  friend void swap(Array &x, Array &y) noexcept;
  Array *clone() const final;
  virtual ~Array() = default;

  mpz_class width() const final;
  mpz_class count() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct TypeExprID : public TypeExpr {

  std::string name;
  std::shared_ptr<TypeExpr> referent;

  TypeExprID() = delete;
  TypeExprID(const std::string &name_, std::shared_ptr<TypeExpr> referent_, const location &loc_);
  TypeExprID(const TypeExprID &other);
  TypeExprID &operator=(TypeExprID other);
  friend void swap(TypeExprID &x, TypeExprID &y) noexcept;
  TypeExprID *clone() const final;
  virtual ~TypeExprID() = default;

  mpz_class width() const final;
  mpz_class count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;
  const TypeExpr *resolve() const final;
  void validate() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;
};

}
