#include <rumur/Decl.h>
#include <rumur/Function.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>
#include "vector_utils.h"

namespace rumur {

Parameter::Parameter(VarDecl *decl_, bool by_reference_, const location &loc_):
  Node(loc_), decl(decl_), by_reference(by_reference_) { }

Parameter::Parameter(const Parameter &other):
  Node(other), decl(other.decl->clone()), by_reference(other.by_reference) { }

Parameter &Parameter::operator=(Parameter other) {
  swap(*this, other);
  return *this;
}

void swap(Parameter &x, Parameter &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.decl, y.decl);
  swap(x.by_reference, y.by_reference);
}

Parameter::~Parameter() {
  delete decl;
}

Parameter *Parameter::clone() const {
  return new Parameter(*this);
}

bool Parameter::operator==(const Node &other) const {
  auto o = dynamic_cast<const Parameter*>(&other);
  if (o == nullptr)
    return false;
  if (*decl != *o->decl)
    return false;
  if (by_reference != o->by_reference)
    return false;
  return true;
}

Function::Function(const std::string &name_,
  std::vector<Parameter*> &&parameters_, TypeExpr *return_type_,
  std::vector<Decl*> &&decls_, std::vector<Stmt*> &&body_,
  const location &loc_):
  Node(loc_), name(name_), parameters(parameters_), return_type(return_type_),
  decls(decls_), body(body_) { }

Function::Function(const Function &other):
  Node(other), name(other.name),
  return_type(other.return_type == nullptr ? nullptr : other.return_type->clone()) {

  for (const Parameter *p : other.parameters)
    parameters.push_back(p->clone());

  for (const Decl *d : other.decls)
    decls.push_back(d->clone());

  for (const Stmt *s : other.body)
    body.push_back(s->clone());
}

Function &Function::operator=(Function other) {
  swap(*this, other);
  return *this;
}

void swap(Function &x, Function &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.parameters, y.parameters);
  swap(x.return_type, y.return_type);
  swap(x.decls, y.decls);
  swap(x.body, y.body);
}

Function::~Function() {
  for (Parameter *p : parameters)
    delete p;
  delete return_type;
  for (Decl *d : decls)
    delete d;
  for (Stmt *s : body)
    delete s;
}

Function *Function::clone() const {
  return new Function(*this);
}

bool Function::operator==(const Node &other) const {
  auto o = dynamic_cast<const Function*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (!vector_eq(parameters, o->parameters))
    return false;
  if (return_type == nullptr) {
    if (o->return_type != nullptr)
      return false;
  } else {
    if (o->return_type == nullptr)
      return false;
    if (*return_type != *o->return_type)
      return false;
  }
  if (!vector_eq(decls, o->decls))
    return false;
  if (!vector_eq(body, o->body))
    return false;
  return true;
}

}
