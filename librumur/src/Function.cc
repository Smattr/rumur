#include <rumur/Decl.h>
#include <rumur/Function.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

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
  std::vector<Parameter*> &&parameters_, std::vector<Decl*> &&decls_,
  TypeExpr *return_type_, const location &loc_):
  Node(loc_), name(name_), parameters(parameters_), decls(decls_),
  return_type(return_type_) { }

Function::Function(const Function &other):
  Node(other), name(other.name),
  return_type(other.return_type == nullptr ? nullptr : other.return_type->clone()) {

  for (const Parameter *p : other.parameters)
    parameters.push_back(p->clone());

  for (const Decl *d : other.decls)
    decls.push_back(d->clone());
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
  swap(x.decls, y.decls);
  swap(x.return_type, y.return_type);
}

Function::~Function() {
  for (Parameter *p : parameters)
    delete p;
  for (Decl *d : decls)
    delete d;
  delete return_type;
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
  for (auto it = parameters.begin(); ; it++) {
    for (auto it2 = o->parameters.begin(); ; it2++) {
      if (it == parameters.end()) {
        if (it2 != o->parameters.end())
          return false;
        break;
      }
      if (it2 == o->parameters.end())
        return false;
      if (*it != *it2)
        return false;
    }
  }
  for (auto it = decls.begin(); ; it++) {
    for (auto it2 = o->decls.begin(); ; it2++) {
      if (it == decls.end()) {
        if (it2 != o->decls.end())
          return false;
        break;
      }
      if (it2 == o->decls.end())
        return false;
      if (*it != *it2)
        return false;
    }
  }
  if (return_type == nullptr) {
    if (o->return_type != nullptr)
      return false;
  } else {
    if (o->return_type == nullptr)
      return false;
    if (*return_type != *o->return_type)
      return false;
  }
  return true;
}

}
