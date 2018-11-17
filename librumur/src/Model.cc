#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Function.h>
#include <rumur/indexer.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <unordered_set>
#include "utils.h"
#include <vector>

namespace rumur {

Model::Model(const std::vector<Ptr<Decl>> &decls_,
  const std::vector<Ptr<Function>> &functions_,
  const std::vector<Ptr<Rule>> &rules_, const location &loc_):
  Node(loc_), decls(decls_), functions(functions_), rules(rules_) { }

Model::Model(const Model &other):
  Node(other), decls(other.decls), functions(other.functions),
  rules(other.rules) { }

Model &Model::operator=(Model other) {
  swap(*this, other);
  return *this;
}

void swap(Model &x, Model &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.decls, y.decls);
  swap(x.functions, y.functions);
  swap(x.rules, y.rules);
}

Model *Model::clone() const {
  return new Model(*this);
}

mpz_class Model::size_bits() const {
  mpz_class s = 0;
  for (const Ptr<Decl> &d : decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d.get()))
      s += v->type->width();
  }
  return s;
}

bool Model::operator==(const Node &other) const {
  auto o = dynamic_cast<const Model*>(&other);
  if (o == nullptr)
    return false;
  if (!vector_eq(decls, o->decls))
    return false;
  if (!vector_eq(functions, o->functions))
    return false;
  if (!vector_eq(rules, o->rules))
    return false;
  return true;
}

void Model::validate() const {

  // Check all state variable names are distinct.
  {
    std::unordered_set<std::string> names;
    for (const Ptr<Decl> &d : decls) {
      if (auto v = dynamic_cast<const VarDecl*>(d.get())) {
        if (!names.insert(v->name).second)
          throw Error("duplicate state variable name \"" + v->name + "\"",
            v->loc);
      }
    }
  }

  // Check all rule names are distinct.
  {
    std::unordered_set<std::string> names;
    for (const Ptr<Rule> &r : rules) {
      if (r->name != "") {
        if (!names.insert(r->name).second)
          throw Error("duplicate rule name " + r->name, r->loc);
      }
    }
  }
}

unsigned long Model::assumption_count() const {

  // Define a traversal for counting the assumptions encountered.
  class AssumptionCounter : public ConstTraversal {

   public:
    unsigned long assumptions = 0;

    void visit(const Property &n) final {
      if (n.category == Property::ASSUMPTION)
        assumptions++;
      dispatch(*n.expr);
    }

    virtual ~AssumptionCounter() { }
  };

  // Use the traversal to count our own assumptions.
  AssumptionCounter ac;
  ac.dispatch(*this);

  return ac.assumptions;
}

void Model::reindex() {
  mpz_class offset = 0;
  for (Ptr<Decl> &d : decls) {
    if (auto v = dynamic_cast<VarDecl*>(d.get())) {
      v->offset = offset;
      offset += v->type->width();
    }
  }

  // Re-number our and our children's 'unique_id' members
  Indexer i;
  i.dispatch(*this);
}

}
