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

Rule::Rule(const std::string &name, Expr *guard, std::vector<Decl*> &&decls, std::vector<Stmt*> &&body,
  const location &loc):
  Node(loc), name(name), guard(guard), decls(decls), body(body) {
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
    << "[](const State &s [[gnu::unused]]){return ";
  if (guard == nullptr) {
    out << "true";
  } else {
    out << *guard;
  }
  out << ";},"

    // body
    << "[](State &s){";
  // TODO: decls
  for (const Stmt *s : body)
    out << *s << ";";
  out << "})";
}

Rule::~Rule() {
  delete guard;
  for (Decl *d : decls)
    delete d;
  for (Stmt *s : body)
    delete s;
}

/* XXX: Why do we need the explicit moves here? I assumed that we could
 * transparently forward the rvalue references, but it seems they need to be
 * bound to lvalue references which does not happen automatically.
 */
StartState::StartState(const std::string &name, std::vector<Decl*> &&decls,
  std::vector<Stmt*> &&body, const location &loc):
  Rule(name, nullptr, move(decls), move(body), loc) {
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

}
