#include <iostream>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <string>
#include <vector>

using namespace std;

namespace rumur {

Rule::Rule(const string &name, Expr *guard, vector<Decl*> &&decls, vector<Stmt*> &&body,
  const location &loc, Indexer&)
  : Node(loc), name(name), guard(guard), decls(decls), body(body) {
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

void Rule::generate_rule(ostream &out) const {
    // TODO: decls
    for (const Stmt *s : body) {
        s->generate_stmt(out);
        out << "\n";
    }
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
StartState::StartState(const string &name, vector<Decl*> &&decls,
  vector<Stmt*> &&body, const location &loc, Indexer& indexer)
  : Rule(name, nullptr, move(decls), move(body), loc, indexer) {
}

StartState *StartState::clone() const {
    return new StartState(*this);
}

Invariant::Invariant(const string &name_, Expr *guard_,
  const location &loc_, Indexer&)
  : Node(loc_), name(name_), guard(guard_) {
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

}
