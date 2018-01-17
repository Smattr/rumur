#include <algorithm>
#include <cstdint>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Rule.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace rumur {

Model::Model(std::vector<Decl*> &&decls, std::vector<Rule*> &&rules, const location &loc):
  Node(loc), decls(decls), rules(rules) {
}

Model::Model(const Model &other):
  Node(other) {
  for (const Decl *d : other.decls)
    decls.push_back(d->clone());
  for (const Rule *r : other.rules)
    rules.push_back(r->clone());
}

Model &Model::operator=(Model other) {
  swap(*this, other);
  return *this;
}

void swap(Model &x, Model &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.decls, y.decls);
  swap(x.rules, y.rules);
}

Model *Model::clone() const {
  return new Model(*this);
}

void Model::validate() const {

  for (const Decl *d : decls)
    d->validate();

  // Check we have at least one start state.
  auto is_start_state = [](const Rule *r) {
    return dynamic_cast<const StartState*>(r) != nullptr;
  };
  if (find_if(rules.begin(), rules.end(), is_start_state) == rules.end())
    throw RumurError("model has no start state", location());

  // Check all rule names are distinct.
  std::unordered_set<std::string> names;
  for (const Rule *r : rules) {
    if (r->name != "") {
      if (!names.insert(r->name).second)
        throw RumurError("duplicate rule name " + r->name, r->loc);
    }
  }

}

uint64_t Model::size_bits() const {
  // TODO
  return 1;
}

Model::~Model() {
  for (Decl *d : decls)
    delete d;
  for (Rule *r : rules)
    delete r;
}

void Model::generate(std::ostream &) const {
  // TODO
}

}
