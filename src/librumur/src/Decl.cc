#include <iostream>
#include "location.hh"
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

ConstDecl::ConstDecl(const std::string &name_, const Expr *value_,
  const location &loc_):
  Decl(name_, loc_), value(value_->clone()) {
}

ConstDecl::ConstDecl(const ConstDecl &other):
  Decl(other), value(other.value->clone()) {
}

ConstDecl &ConstDecl::operator=(ConstDecl other) {
  swap(*this, other);
  return *this;
}

void swap(ConstDecl &x, ConstDecl &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.value, y.value);
}

ConstDecl *ConstDecl::clone() const {
  return new ConstDecl(*this);
}

void ConstDecl::validate() const {
  value->validate();
  if (!value->constant())
    throw RumurError("const definition is not a constant", value->loc);
}

void ConstDecl::generate(std::ostream &out) const {
  int64_t v = value->constant_fold();
  out << "static const Number ru_u_" << name << "(INT64_C(" << v << "))";
}

ConstDecl::~ConstDecl() {
  delete value;
}

TypeDecl::TypeDecl(const std::string &name_, TypeExpr *value_,
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
  swap(x.name, y.name);
  swap(x.value, y.value);
}

TypeDecl *TypeDecl::clone() const {
  return new TypeDecl(*this);
}

void TypeDecl::validate() const {
  value->validate();
}

TypeDecl::~TypeDecl() {
  delete value;
}

void TypeDecl::generate(std::ostream &out) const {
  out << "using ru_u_" << name << " = " << *value;
}

VarDecl::VarDecl(const std::string &name_, TypeExpr *type_,
  const location &loc_):
  Decl(name_, loc_), type(type_) {
}

VarDecl::VarDecl(const VarDecl &other):
  Decl(other), type(other.type->clone()), state_variable(other.state_variable) {
}

VarDecl &VarDecl::operator=(VarDecl other) {
  swap(*this, other);
  return *this;
}

void swap(VarDecl &x, VarDecl &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.type, y.type);
  swap(x.state_variable, y.state_variable);
}

VarDecl *VarDecl::clone() const {
  return new VarDecl(*this);
}

void VarDecl::generate(std::ostream &out) const {
  out << "using ru_u_" << name << " = " << *type;
}

VarDecl::~VarDecl() {
  delete type;
}

}
