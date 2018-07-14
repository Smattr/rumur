#pragma once

#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// Forward declare to avoid circular #include
class VarDecl;

class TypeExpr : public Node {

 public:
  using Node::Node;
  TypeExpr() = delete;
  TypeExpr(const TypeExpr&) = default;
  TypeExpr(TypeExpr&&) = default;
  TypeExpr &operator=(const TypeExpr&) = default;
  TypeExpr &operator=(TypeExpr&&) = default;
  virtual ~TypeExpr() { }

  // Whether this type is a primitive integer-like type.
  virtual bool is_simple() const;

  TypeExpr *clone() const override = 0;
  virtual size_t width() const;
  virtual size_t count() const = 0;
  virtual const TypeExpr *resolve() const;

  /* Numeric bounds of this type as valid C code. These are only valid to use on
   * TypeExprs for which is_simple() returns true.
   */
  virtual std::string lower_bound() const;
  virtual std::string upper_bound() const;

  virtual void generate_print(std::ostream &out, std::string const &prefix = "",
    size_t preceding_offset = 0) const = 0;
};

class Range : public TypeExpr {

 public:
  Expr *min;
  Expr *max;

  Range() = delete;
  Range(Expr *min_, Expr *max_, const location &loc_);
  Range(const Range &other);
  Range &operator=(Range other);
  friend void swap(Range &x, Range &y) noexcept;
  Range *clone() const final;
  virtual ~Range();

  size_t count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;

  void generate_print(std::ostream &out, std::string const &prefix = "",
    size_t preceding_offset = 0) const final;
};

class Scalarset : public TypeExpr {

 public:
  Expr *bound;

  Scalarset() = delete;
  Scalarset(Expr *bound_, const location &loc_);
  Scalarset(const Scalarset &other);
  Scalarset &operator=(Scalarset other);
  friend void swap(Scalarset &x, Scalarset &y) noexcept;
  Scalarset *clone() const final;
  virtual ~Scalarset();

  size_t count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;

  void generate_print(std::ostream &out, std::string const &prefix = "",
    size_t preceding_offset = 0) const final;
};

class Enum : public TypeExpr {

 public:
  std::vector<std::pair<std::string, location>> members;

  Enum() = delete;
  Enum(const std::vector<std::pair<std::string, location>> &&members_,
    const location &loc_);
  Enum(const Enum&) = default;
  Enum(Enum&&) = default;
  Enum &operator=(const Enum&) = default;
  Enum &operator=(Enum&&) = default;
  Enum *clone() const final;
  virtual ~Enum() { }

  size_t count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;

  void generate_print(std::ostream &out, std::string const &prefix = "",
    size_t preceding_offset = 0) const final;
};

class Record : public TypeExpr {

 public:
  std::vector<VarDecl*> fields;

  Record() = delete;
  Record(std::vector<VarDecl*> &&fields_, const location &loc_);
  Record(const Record &other);
  Record &operator=(Record other);
  friend void swap(Record &x, Record &y) noexcept;
  Record *clone() const final;
  virtual ~Record();

  size_t width() const final;
  size_t count() const final;
  bool operator==(const Node &other) const final;

  void generate_print(std::ostream &out, std::string const &prefix = "",
    size_t preceding_offset = 0) const final;
};

class Array : public TypeExpr {

 public:
  TypeExpr *index_type;
  TypeExpr *element_type;

  Array() = delete;
  Array(TypeExpr *index_type_, TypeExpr *element_type_, const location &loc_);
  Array(const Array &other);
  Array &operator=(Array other);
  friend void swap(Array &x, Array &y) noexcept;
  Array *clone() const final;
  virtual ~Array();

  size_t width() const final;
  size_t count() const final;
  bool operator==(const Node &other) const final;

  void generate_print(std::ostream &out, std::string const &prefix = "",
    size_t preceding_offset = 0) const final;
};

class TypeExprID : public TypeExpr {

 public:
  std::string name;
  TypeExpr *referent;

  TypeExprID() = delete;
  TypeExprID(const std::string &name_, TypeExpr *referent_, const location &loc_);
  TypeExprID(const TypeExprID &other);
  TypeExprID &operator=(TypeExprID other);
  friend void swap(TypeExprID &x, TypeExprID &y) noexcept;
  TypeExprID *clone() const final;
  virtual ~TypeExprID();

  size_t width() const final;
  size_t count() const final;
  bool operator==(const Node &other) const final;
  bool is_simple() const final;
  const TypeExpr *resolve() const final;

  std::string lower_bound() const final;
  std::string upper_bound() const final;

  void generate_print(std::ostream &out, std::string const &prefix = "",
    size_t preceding_offset = 0) const final;
};

}
