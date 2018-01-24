#include <cassert>
#include <iostream>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <string>
#include <vector>

namespace rumur {

Rule::Rule(const std::string &name_, Expr *guard_, std::vector<Decl*> &&decls_,
  std::vector<Stmt*> &&body_, const location &loc_):
  Node(loc_), name(name_), guard(guard_), decls(decls_), body(body_) {
}

Rule::Rule(const Rule &other):
  Node(other), name(other.name), guard(other.guard == nullptr ? nullptr : other.guard->clone()) {
  for (const Decl *d : other.decls)
    decls.push_back(d->clone());
  for (const Stmt *s : other.body)
    body.push_back(s->clone());
}

Rule &Rule::operator=(Rule other) {
  swap(*this, other);
  return *this;
}

void swap(Rule &x, Rule &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.decls, y.decls);
  swap(x.body, y.body);
}

Rule *Rule::clone() const {
  return new Rule(*this);
}

void Rule::generate(std::ostream &out) const {
  out << "Rule(\"" << name << "\","
  
    // guard
    << "[](const State &s [[gnu::unused]]){\n"
    << "return ";
  if (guard == nullptr) {
    out << "true";
  } else {
    out << *guard;
  }
  out << ";\n},"

    // body
    << "[](State &s){\n";
  // TODO: decls
  for (const Stmt *s : body)
    out << *s << ";\n";
  out << "})";
}

Rule::~Rule() {
  delete guard;
  for (Decl *d : decls)
    delete d;
  for (Stmt *s : body)
    delete s;
}

bool Rule::operator==(const Node &other) const {
  if (dynamic_cast<const StartState*>(&other) != nullptr)
    return false;
  auto o = dynamic_cast<const Rule*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (guard == nullptr) {
    if (o->guard != nullptr)
      return false;
  } else {
    if (o->guard == nullptr || *guard != *o->guard)
      return false;
  }
  for (auto it = decls.begin(), it2 = o->decls.begin(); ; it++, it2++) {
    if (it == decls.end()) {
      if (it2 != o->decls.end())
        return false;
      break;
    }
    if (it2 == o->decls.end())
      return false;
    if (**it != **it2)
      return false;
  }
  for (auto it = body.begin(), it2 = o->body.begin(); ; it++, it2++) {
    if (it == body.end()) {
      if (it2 != o->body.end())
        return false;
      break;
    }
    if (it2 == o->body.end())
      return false;
    if (**it != **it2)
      return false;
  }
  return true;
}

/* XXX: Why do we need the explicit moves here? I assumed that we could
 * transparently forward the rvalue references, but it seems they need to be
 * bound to lvalue references which does not happen automatically.
 */
StartState::StartState(const std::string &name_, std::vector<Decl*> &&decls_,
  std::vector<Stmt*> &&body_, const location &loc_):
  Rule(name_, nullptr, std::move(decls_), std::move(body_), loc_) {
}

StartState *StartState::clone() const {
  return new StartState(*this);
}

void StartState::generate(std::ostream &out) const {
  out << "StartState(\"" << name << "\","

    // body
    << "[](State &s){\n";
  // TODO: decls
  for (const Stmt *s : body)
    out << *s << ";\n";
  out << "})";
}

bool StartState::operator==(const Node &other) const {
  auto o = dynamic_cast<const StartState*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (guard == nullptr) {
    if (o->guard != nullptr)
      return false;
  } else {
    if (o->guard == nullptr || *guard != *o->guard)
      return false;
  }
  for (auto it = decls.begin(), it2 = o->decls.begin(); ; it++, it2++) {
    if (it == decls.end()) {
      if (it2 != o->decls.end())
        return false;
      break;
    }
    if (it2 == o->decls.end())
      return false;
    if (**it != **it2)
      return false;
  }
  for (auto it = body.begin(), it2 = o->body.begin(); ; it++, it2++) {
    if (it == body.end()) {
      if (it2 != o->body.end())
        return false;
      break;
    }
    if (it2 == o->body.end())
      return false;
    if (**it != **it2)
      return false;
  }
  return true;
}

Invariant::Invariant(const std::string &name_, Expr *guard_,
  const location &loc_):
  Node(loc_), name(name_), guard(guard_) {
}

Invariant::Invariant(const Invariant &other):
  Node(other), name(other.name), guard(other.guard->clone()) {
}

Invariant &Invariant::operator=(Invariant other) {
  swap(*this, other);
  return *this;
}

void swap(Invariant &x, Invariant &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.guard, y.guard);
}

Invariant *Invariant::clone() const {
  return new Invariant(*this);
}

Invariant::~Invariant() {
  delete guard;
}

void Invariant::generate(std::ostream &out) const {
  assert(guard != nullptr && "BUG: invariant with no body");

  out << "Invariant(\"" << name << "\","
  
    // guard
    << "[](const State &s){return "
    << *guard
    << ";})";
}

bool Invariant::operator==(const Node &other) const {
  auto o = dynamic_cast<const Invariant*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (guard == nullptr) {
    if (o->guard != nullptr)
      return false;
  } else {
    if (o->guard == nullptr || *guard != *o->guard)
      return false;
  }
  return true;
}

}
