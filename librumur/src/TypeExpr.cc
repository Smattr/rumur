#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <limits.h>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Number.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <unordered_set>
#include <utility>
#include "utils.h"
#include <vector>

namespace rumur {

bool TypeExpr::is_simple() const {
  return false;
}

const TypeExpr *TypeExpr::resolve() const {
  return this;
}

std::string TypeExpr::lower_bound() const {
  throw Error("complex types do not have valid lower bounds", loc);
}

std::string TypeExpr::upper_bound() const {
  throw Error("complex types do not have valid upper bounds", loc);
}

mpz_class TypeExpr::width() const {
  mpz_class c = count();

  // If there are 0 or 1 values of this type, its width is trivial.
  if (c <= 1)
    return 0;

  /* Otherwise, we need the number of bits required to represent the largest
   * value.
   */
  mpz_class largest(c - 1);
  mpz_class bits(0);
  while (largest != 0) {
    bits++;
    largest >>= 1;
  }
  return bits;
}

Range::Range(const Ptr<Expr> &min_, const Ptr<Expr> &max_,
  const location &loc_):
  TypeExpr(loc_), min(min_), max(max_) {

  if (min == nullptr) {
    // FIXME: avoid hard coding INT64 limits here
    // FIXME: this isn't even the right limit because of - overflowing grr...
    min = Ptr<Number>::make(mpz_class("-9223372036854775807"), location());
  }

  if (max == nullptr) {
    max = Ptr<Number>::make(mpz_class("9223372036854775807"), location());
  }
}

Range::Range(const Range &other):
  TypeExpr(other), min(other.min), max(other.max) {
}

Range &Range::operator=(Range other) {
  swap(*this, other);
  return *this;
}

void swap(Range &x, Range &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.min, y.min);
  swap(x.max, y.max);
}

Range *Range::clone() const {
  return new Range(*this);
}

mpz_class Range::count() const {
  mpz_class lb = min->constant_fold();
  mpz_class ub = max->constant_fold();
  return ub - lb + 2;
}

bool Range::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Range*>(&other))
    return min->constant_fold() == o->min->constant_fold() &&
           max->constant_fold() == o->max->constant_fold();

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *o == *this;

  return false;
}

bool Range::is_simple() const {
  return true;
}

void Range::validate() const {
  if (!min->constant())
    throw Error("lower bound of range is not a constant", min->loc);

  if (!max->constant())
    throw Error("upper bound of range is not a constant", max->loc);
}

std::string Range::lower_bound() const {
  return "VALUE_C(" + min->constant_fold().get_str() + ")";
}

std::string Range::upper_bound() const {
  return "VALUE_C(" + max->constant_fold().get_str() + ")";
}

Scalarset::Scalarset(const Ptr<Expr> &bound_, const location &loc_):
  TypeExpr(loc_), bound(bound_) { }

Scalarset::Scalarset(const Scalarset &other):
  TypeExpr(other), bound(other.bound) { }

Scalarset &Scalarset::operator=(Scalarset other) {
  swap(*this, other);
  return *this;
}

void swap(Scalarset &x, Scalarset &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.bound, y.bound);
}

Scalarset *Scalarset::clone() const {
  return new Scalarset(*this);
}

mpz_class Scalarset::count() const {
  mpz_class b = bound->constant_fold();
  assert(b > 0 && "non-positive bound for scalarset");

  return b + 1;
}

bool Scalarset::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Scalarset*>(&other))
    return bound->constant_fold() == o->bound->constant_fold();

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *o == *this;

  return false;
}

bool Scalarset::is_simple() const {
  return true;
}

void Scalarset::validate() const {
  if (!bound->constant())
    throw Error("bound of scalarset is not a constant", bound->loc);

  if (bound->constant_fold() <= 0)
    throw Error("bound of scalarset is not positive", bound->loc);
}

std::string Scalarset::lower_bound() const {
  return "VALUE_C(0)";
}

std::string Scalarset::upper_bound() const {
  mpz_class b = bound->constant_fold() - 1;
  return "VALUE_C(" + b.get_str() + ")";
}

Enum::Enum(const std::vector<std::pair<std::string, location>> &&members_, const location &loc_):
  TypeExpr(loc_), members(members_) {
}

Enum *Enum::clone() const {
  return new Enum(*this);
}

mpz_class Enum::count() const {
  mpz_class members_size = members.size();
  return members_size + 1;
}

bool Enum::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Enum*>(&other)) {
    for (auto it = members.begin(), it2 = o->members.begin(); ; it++, it2++) {
      if (it == members.end()) {
        if (it2 != o->members.end())
          return false;
        break;
      }
      if (it2 == o->members.end())
        return false;
      if ((*it).first != (*it2).first)
        return false;
    }
    return true;
  }

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *o == *this;

  return false;
}

bool Enum::is_simple() const {
  return true;
}

std::string Enum::lower_bound() const {
  return "0";
}

std::string Enum::upper_bound() const {
  mpz_class size = members.size();
  if (size > 0)
    size--;
  return "VALUE_C(" + size.get_str() + ")";
}

Record::Record(const std::vector<Ptr<VarDecl>> &fields_,
  const location &loc_):
  TypeExpr(loc_), fields(fields_) {

  std::unordered_set<std::string> names;
  for (const Ptr<VarDecl> &f : fields) {
    if (!names.insert(f->name).second)
      throw Error("duplicate field name \"" + f->name + "\"", f->loc);
  }
}

Record::Record(const Record &other):
  TypeExpr(other), fields(other.fields) { }

Record &Record::operator=(Record other) {
  swap(*this, other);
  return *this;
}

void swap(Record &x, Record &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.fields, y.fields);
}

Record *Record::clone() const {
  return new Record(*this);
}

mpz_class Record::width() const {
  mpz_class s = 0;
  for (const Ptr<VarDecl> &v : fields)
    s += v->type->width();
  return s;
}

mpz_class Record::count() const {
  mpz_class s = 1;
  for (const Ptr<VarDecl> &v : fields)
    s *= v->type->count();
  return s;
}

bool Record::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Record*>(&other)) {
    if (!vector_eq(fields, o->fields))
      return false;
    return true;
  }

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *o == *this;

  return false;
}

Array::Array(std::shared_ptr<TypeExpr> index_type_,
  std::shared_ptr<TypeExpr> element_type_, const location &loc_):
  TypeExpr(loc_), index_type(index_type_), element_type(element_type_) { }

Array::Array(const Array &other):
  TypeExpr(other), index_type(other.index_type->clone()),
  element_type(other.element_type->clone()) { }

Array &Array::operator=(Array other) {
  swap(*this, other);
  return *this;
}

void swap(Array &x, Array &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.index_type, y.index_type);
  swap(x.element_type, y.element_type);
}

Array *Array::clone() const {
  return new Array(*this);
}

mpz_class Array::width() const {

  mpz_class i = index_type->count();
  mpz_class e = element_type->width();

  assert(i >= 1 && "index count apparently does not include undefined");
  i--;

  return i * e;
}

mpz_class Array::count() const {

  mpz_class i = index_type->count();
  mpz_class e = element_type->count();

  assert(i >= 1 && "index count apparently does not include undefined");
  i--;

  if (i == 0)
    return 0;

  mpz_class s = 1;
  for (size_t j = 0; j < i; j++)
    s *= e;
  return s;
}

bool Array::operator==(const Node &other) const {
  if (auto o = dynamic_cast<const Array*>(&other))
    return *index_type == *o->index_type
        && *element_type == *o->element_type;

  if (auto o = dynamic_cast<const TypeExprID*>(&other))
    return *o == *this;

  return false;
}

void Array::validate() const {
  if (!index_type->is_simple())
    throw Error("array indices must be simple types", loc);
}

TypeExprID::TypeExprID(const std::string &name_,
  std::shared_ptr<TypeExpr> referent_, const location &loc_):
  TypeExpr(loc_), name(name_), referent(referent_) { }

TypeExprID::TypeExprID(const TypeExprID &other):
  TypeExpr(other.loc), name(other.name), referent(other.referent) { }

TypeExprID &TypeExprID::operator=(TypeExprID other) {
  swap(*this, other);
  return *this;
}

void swap(TypeExprID &x, TypeExprID &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.name, y.name);
  swap(x.referent, y.referent);
}

TypeExprID *TypeExprID::clone() const {
  return new TypeExprID(*this);
}

mpz_class TypeExprID::width() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->width();
}

mpz_class TypeExprID::count() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->count();
}

bool TypeExprID::operator==(const Node &other) const {
  if (referent != nullptr)
    return *referent == other;

  auto o = dynamic_cast<const TypeExprID*>(&other);
  if (o == nullptr)
    return false;

  if (o->referent != nullptr)
    return false;

  return true;
}

bool TypeExprID::is_simple() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->is_simple();
}

const TypeExpr *TypeExprID::resolve() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->resolve();
}

void TypeExprID::validate() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
}

std::string TypeExprID::lower_bound() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->lower_bound();
}

std::string TypeExprID::upper_bound() const {
  if (referent == nullptr)
    throw Error("unresolved type symbol \"" + name + "\"", loc);
  return referent->upper_bound();
}

}
