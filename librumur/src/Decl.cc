#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <string>

namespace rumur {

Decl::Decl(const std::string &name_, const location &loc_):
  Node(loc_), name(name_) {
}

Decl::~Decl() {
}

AliasDecl::AliasDecl(const std::string &name_, std::shared_ptr<Expr> value_,
    const location &loc_):
  ExprDecl(name_, loc_), value(value_) { }

AliasDecl::AliasDecl(const AliasDecl &other):
  ExprDecl(other), value(other.value->clone()) { }

AliasDecl &AliasDecl::operator=(AliasDecl other) {
  swap(*this, other);
  return *this;
}

void swap(AliasDecl &x, AliasDecl &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.name, y.name);
  swap(x.value, y.value);
}

AliasDecl *AliasDecl::clone() const {
  return new AliasDecl(*this);
}

bool AliasDecl::operator==(const Node &other) const {
  auto o = dynamic_cast<const AliasDecl*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (*value != *o->value)
    return false;
  return true;
}

bool AliasDecl::is_lvalue() const {
  return value->is_lvalue();
}

ConstDecl::ConstDecl(const std::string &name_, std::shared_ptr<Expr> value_,
  const location &loc_):
  ExprDecl(name_, loc_), value(value_) { }

ConstDecl::ConstDecl(const std::string &name_, std::shared_ptr<Expr> value_,
  std::shared_ptr<TypeExpr> type_, const location &loc_):
  ExprDecl(name_, loc_), value(value_), type(type_) { }

ConstDecl::ConstDecl(const ConstDecl &other):
  ExprDecl(other), value(other.value->clone()), type(other.type) { }

ConstDecl &ConstDecl::operator=(ConstDecl other) {
  swap(*this, other);
  return *this;
}

void swap(ConstDecl &x, ConstDecl &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.name, y.name);
  swap(x.value, y.value);
  swap(x.type, y.type);
}

ConstDecl *ConstDecl::clone() const {
  return new ConstDecl(*this);
}

bool ConstDecl::operator==(const Node &other) const {
  auto o = dynamic_cast<const ConstDecl*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (*value != *o->value)
    return false;
  if (type == nullptr) {
    if (o->type != nullptr)
      return false;
  } else {
    if (o->type == nullptr)
      return false;
    if (*type != *o->type)
      return false;
  }
  return true;
}

bool ConstDecl::is_lvalue() const {
  return false;
}

void ConstDecl::validate() const {
  if (!value->constant())
    throw Error("const definition is not a constant", value->loc);
}

TypeDecl::TypeDecl(const std::string &name_, std::shared_ptr<TypeExpr> value_,
  const location &loc_):
  Decl(name_, loc_), value(value_) {
}

TypeDecl::TypeDecl(const TypeDecl &other):
  Decl(other), value(other.value->clone()) {
}

TypeDecl &TypeDecl::operator=(TypeDecl other) {
  swap(*this, other);
  return *this;
}

void swap(TypeDecl &x, TypeDecl &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.name, y.name);
  swap(x.value, y.value);
}

TypeDecl *TypeDecl::clone() const {
  return new TypeDecl(*this);
}

bool TypeDecl::operator==(const Node &other) const {
  auto o = dynamic_cast<const TypeDecl*>(&other);
  return o != nullptr && name == o->name && *value == *o->value;
}

VarDecl::VarDecl(const std::string &name_, std::shared_ptr<TypeExpr> type_,
  const location &loc_):
  ExprDecl(name_, loc_), type(type_) {
}

VarDecl::VarDecl(const VarDecl &other):
  ExprDecl(other), type(other.type->clone()), offset(other.offset),
  readonly(other.readonly) { }

VarDecl &VarDecl::operator=(VarDecl other) {
  swap(*this, other);
  return *this;
}

void swap(VarDecl &x, VarDecl &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.name, y.name);
  swap(x.type, y.type);
  swap(x.offset, y.offset);
  swap(x.readonly, y.readonly);
}

VarDecl *VarDecl::clone() const {
  return new VarDecl(*this);
}

bool VarDecl::operator==(const Node &other) const {
  auto o = dynamic_cast<const VarDecl*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (*type != *o->type)
    return false;
  if (offset != o->offset)
    return false;
  if (readonly != o->readonly)
    return false;
  return true;
}

bool VarDecl::is_lvalue() const {
  return !readonly;
}

mpz_class VarDecl::width() const {
  return type->width();
}

mpz_class VarDecl::count() const {
  return type->count();
}

}
