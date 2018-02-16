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

Model::Model(std::vector<Decl*> &&decls_, std::vector<Rule*> &&rules_, const location &loc_):
  Node(loc_), decls(decls_), rules(rules_) {

  // Check we have at least one start state.
  auto is_start_state = [](const Rule *r) {
    return dynamic_cast<const StartState*>(r) != nullptr;
  };
  if (find_if(rules.begin(), rules.end(), is_start_state) == rules.end())
    throw Error("model has no start state", location());

  // Check all rule names are distinct.
  std::unordered_set<std::string> names;
  for (const Rule *r : rules) {
    if (r->name != "<unnamed>") {
      if (!names.insert(r->name).second)
        throw Error("duplicate rule name " + r->name, r->loc);
    }
  }
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

uint64_t Model::size_bits() const {
  size_t s = 0;
  for (const Decl *d : decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d))
      s += v->type->width();
  }
  return s;
}

Model::~Model() {
  for (Decl *d : decls)
    delete d;
  for (Rule *r : rules)
    delete r;
}

void Model::generate(std::ostream &out) const {

  out

    // Specialise classes
    << "using State = StateBase<" << size_bits() << ", THREADS>;\n"
    << "using StartState = StartStateBase<State>;\n"
    << "using Invariant = InvariantBase<State>;\n"
    << "using Rule = RuleBase<State>;\n\n";

  // Write out constants and type declarations.
  for (const Decl *d : decls)
    out << *d << ";\n";
  out << "\n";

  // Write out the start state rules.
  out << "static const std::vector<StartState> START_RULES = {\n";
  for (const Rule *r : rules) {
    if (auto s = dynamic_cast<const StartState*>(r))
      out << *s << ",\n";
  }
  out << "};\n\n";

  // Write out the invariant rules.
  out << "static const std::vector<Invariant> INVARIANTS = {\n";
  for (const Rule *r : rules) {
    if (auto i = dynamic_cast<const Invariant*>(r))
      out << *i << ",\n";
  }
  out << "};\n\n";

  // Write out the regular rules.
  out << "static const std::vector<Rule> RULES = {\n";
  for (const Rule *r : rules) {
    if (auto s = dynamic_cast<const SimpleRule*>(r))
      out << *s << ",\n";
  }
  out << "};\n\n";

  // Write a function to print the state.
  out << "static void print_state(const State &s) {\n";
  for (const Decl *d : decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d)) {
      out << "  ru_u_" << v->name << "::make(s, size_t(" << v->offset
        << ")).print(stderr, \"" << v->name << "\");\n"
        << "  fprint(stderr, \"\\n\");\n";
    }
  }
  out << "}\n\n";
}

bool Model::operator==(const Node &other) const {
  auto o = dynamic_cast<const Model*>(&other);
  if (o == nullptr)
    return false;
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

}
