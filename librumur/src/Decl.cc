#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <string>

namespace rumur {

Decl::Decl(const std::string &name_, const location &loc_):
  Node(loc_), name(name_) {
}

Decl::~Decl() {
}

AliasDecl::AliasDecl(const std::string &name_, const Ptr<Expr> &value_,
    const location &loc_):
  ExprDecl(name_, loc_), value(value_) { }

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

bool AliasDecl::is_readonly() const {
  return value->is_readonly();
}

const TypeExpr *AliasDecl::get_type() const {
  return value->type();
}

ConstDecl::ConstDecl(const std::string &name_, const Ptr<Expr> &value_,
  const location &loc_):
  ExprDecl(name_, loc_), value(value_) { }

ConstDecl::ConstDecl(const std::string &name_, const Ptr<Expr> &value_,
  const Ptr<TypeExpr> &type_, const location &loc_):
  ExprDecl(name_, loc_), value(value_), type(type_) { }

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

bool ConstDecl::is_readonly() const {
  return true;
}

const TypeExpr *ConstDecl::get_type() const {

  // If this constant has an explicit type (e.g. it's an enum member), use that.
  if (type != nullptr)
    return type.get();

  /* If this doesn't have an explicit type, fall back on the type of the value
   * it points at. This is irrelevant for numerical constants, but important for
   * boolean constants.
   */
  return value->type();
}

void ConstDecl::validate() const {
  if (!value->constant())
    throw Error("const definition is not a constant", value->loc);
}

TypeDecl::TypeDecl(const std::string &name_, const Ptr<TypeExpr> &value_,
  const location &loc_):
  Decl(name_, loc_), value(value_) {
}

TypeDecl *TypeDecl::clone() const {
  return new TypeDecl(*this);
}

bool TypeDecl::operator==(const Node &other) const {
  auto o = dynamic_cast<const TypeDecl*>(&other);
  return o != nullptr && name == o->name && *value == *o->value;
}

VarDecl::VarDecl(const std::string &name_, const Ptr<TypeExpr> &type_,
  const location &loc_):
  ExprDecl(name_, loc_), type(type_) {
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
  return true;
}

bool VarDecl::is_readonly() const {
  return readonly;
}

mpz_class VarDecl::width() const {
  return type->width();
}

mpz_class VarDecl::count() const {
  return type->count();
}

const TypeExpr *VarDecl::get_type() const {
  return type.get();
}

}
