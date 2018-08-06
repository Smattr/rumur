#include <cassert>
#include <iostream>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <string>
#include <vector>

namespace rumur {

namespace {
  /* A traversal pass that checks any return statements within a rule do not
   * have a trailing expression.
   */
  class ReturnChecker : public Traversal {

   public:
    void visit(Return &n) final {
      if (n.expr != nullptr)
        throw Error("return statement in rule or startstate returns a value",
          n.loc);

      // No need to recurse into the return statement's child.
    }

    static void check(Node &n) {
      ReturnChecker c;
      c.dispatch(n);
    }
  };
}

Rule::Rule(const std::string &name_, const location &loc_):
  Node(loc_), name(name_) { }

Rule::Rule(const Rule &other):
  Node(other), name(other.name) {
  for (const Quantifier *q : other.quantifiers)
    quantifiers.push_back(q->clone());
}

std::vector<Rule*> Rule::flatten() const {
  return { clone() };
}

Rule::~Rule() {
  for (Quantifier *q : quantifiers)
    delete q;
}

SimpleRule::SimpleRule(const std::string &name_, Expr *guard_, std::vector<Decl*> &&decls_,
  std::vector<Stmt*> &&body_, const location &loc_):
  Rule(name_, loc_), guard(guard_), decls(decls_), body(body_) {

  ReturnChecker::check(*this);
}

SimpleRule::SimpleRule(const SimpleRule &other):
  Rule(other), guard(other.guard == nullptr ? nullptr : other.guard->clone()) {
  for (const Decl *d : other.decls)
    decls.push_back(d->clone());
  for (const Stmt *s : other.body)
    body.push_back(s->clone());
}

SimpleRule &SimpleRule::operator=(SimpleRule other) {
  swap(*this, other);
  return *this;
}

void swap(SimpleRule &x, SimpleRule &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.decls, y.decls);
  swap(x.body, y.body);
  swap(x.quantifiers, y.quantifiers);
}

SimpleRule *SimpleRule::clone() const {
  return new SimpleRule(*this);
}

SimpleRule::~SimpleRule() {
  delete guard;
  for (Decl *d : decls)
    delete d;
  for (Stmt *s : body)
    delete s;
}

bool SimpleRule::operator==(const Node &other) const {
  auto o = dynamic_cast<const SimpleRule*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  for (auto it = quantifiers.begin(), it2 = o->quantifiers.begin(); ; it++, it2++) {
    if (it == quantifiers.end()) {
      if (it2 != o->quantifiers.end())
        return false;
      break;
    }
    if (it2 == o->quantifiers.end())
      return false;
    if (**it != **it2)
      return false;
  }
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

StartState::StartState(const std::string &name_, std::vector<Decl*> &&decls_,
  std::vector<Stmt*> &&body_, const location &loc_):
  Rule(name_, loc_), decls(decls_), body(body_) {

  ReturnChecker::check(*this);
}

StartState::StartState(const StartState &other):
  Rule(other) {
  for (const Decl *d : other.decls)
    decls.push_back(d->clone());
  for (const Stmt *s : other.body)
    body.push_back(s->clone());
}

StartState &StartState::operator=(StartState other) {
  swap(*this, other);
  return *this;
}

void swap(StartState &x, StartState &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.decls, y.decls);
  swap(x.body, y.body);
  swap(x.quantifiers, y.quantifiers);
}

StartState *StartState::clone() const {
  return new StartState(*this);
}

bool StartState::operator==(const Node &other) const {
  auto o = dynamic_cast<const StartState*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  for (auto it = quantifiers.begin(), it2 = o->quantifiers.begin(); ; it++, it2++) {
    if (it == quantifiers.end()) {
      if (it2 != o->quantifiers.end())
        return false;
      break;
    }
    if (it2 == o->quantifiers.end())
      return false;
    if (**it != **it2)
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

PropertyRule::PropertyRule(const std::string &name_, const Property &property_,
  const location &loc_):
  Rule(name_, loc_), property(property_) {
}

PropertyRule::PropertyRule(const PropertyRule &other):
  Rule(other), property(other.property) {
}

PropertyRule &PropertyRule::operator=(PropertyRule other) {
  swap(*this, other);
  return *this;
}

void swap(PropertyRule &x, PropertyRule &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.property, y.property);
  swap(x.quantifiers, y.quantifiers);
}

PropertyRule *PropertyRule::clone() const {
  return new PropertyRule(*this);
}

bool PropertyRule::operator==(const Node &other) const {
  auto o = dynamic_cast<const PropertyRule*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  for (auto it = quantifiers.begin(), it2 = o->quantifiers.begin(); ; it++, it2++) {
    if (it == quantifiers.end()) {
      if (it2 != o->quantifiers.end())
        return false;
      break;
    }
    if (it2 == o->quantifiers.end())
      return false;
    if (**it != **it2)
      return false;
  }
  if (property != o->property)
    return false;
  return true;
}

Ruleset::Ruleset(std::vector<Quantifier*> &&quantifiers_,
  std::vector<Rule*> &&rules_, const location &loc_):
  Rule("", loc_), rules(rules_) {
  quantifiers = quantifiers_;
}

Ruleset::Ruleset(const Ruleset &other):
  Rule(other) {
  for (const Rule *r : other.rules)
    rules.push_back(r->clone());
}

Ruleset &Ruleset::operator=(Ruleset other) {
  swap(*this, other);
  return *this;
}

void swap(Ruleset &x, Ruleset &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.name, y.name);
  swap(x.quantifiers, y.quantifiers);
  swap(x.rules, y.rules);
}

Ruleset *Ruleset::clone() const {
  return new Ruleset(*this);
}

Ruleset::~Ruleset() {
  for (Rule *r : rules)
    delete r;
}

bool Ruleset::operator==(const Node &other) const {
  auto o = dynamic_cast<const Ruleset*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  for (auto it = quantifiers.begin(), it2 = o->quantifiers.begin(); ; it++, it2++) {
    if (it == quantifiers.end()) {
      if (it2 != o->quantifiers.end())
        return false;
      break;
    }
    if (it2 == o->quantifiers.end())
      return false;
    if (**it != **it2)
      return false;
  }
  for (auto it = rules.begin(), it2 = o->rules.begin(); ; it++, it2++) {
    if (it == rules.end()) {
      if (it2 != o->rules.end())
        return false;
      break;
    }
    if (it2 == o->rules.end())
      return false;
    if (**it != **it2)
      return false;
  }
  return true;
}

std::vector<Rule*> Ruleset::flatten() const {
  std::vector<Rule*> rs;
  for (const Rule *r : rules) {
    for (Rule *f : r->flatten()) {
      for (const Quantifier *q : quantifiers)
        f->quantifiers.insert(f->quantifiers.begin(), q->clone());
      rs.push_back(f);
    }
  }
  return rs;
}

}