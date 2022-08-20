#include "location.hh"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Function.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/TypeExpr.h>
#include <rumur/except.h>
#include <rumur/indexer.h>
#include <rumur/traverse.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace rumur {

Model::Model(const std::vector<Ptr<Node>> &children_, const location &loc_)
    : Node(loc_), children(children_) {}

Model *Model::clone() const { return new Model(*this); }

mpz_class Model::size_bits() const {
  mpz_class s = 0;
  for (const Ptr<Node> &n : children) {
    if (auto v = dynamic_cast<const VarDecl *>(n.get()))
      s += v->type->width();
  }
  return s;
}

void Model::validate() const {

  // Check all state variable names are distinct.
  {
    std::unordered_set<std::string> names;
    for (const Ptr<Node> &c : children) {
      if (auto v = dynamic_cast<const VarDecl *>(c.get())) {
        if (!names.insert(v->name).second)
          throw Error("duplicate state variable name \"" + v->name + "\"",
                      v->loc);
      }
    }
  }
}

void Model::visit(BaseTraversal &visitor) { visitor.visit_model(*this); }

void Model::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_model(*this);
}

mpz_class Model::liveness_count() const {

  // Define a traversal for counting liveness properties.
  class LivenessCounter : public ConstTraversal {

  public:
    mpz_class count = 0;
    mpz_class multiplier = 1;

    void visit_ruleset(const Ruleset &n) final {
      /* Adjust the multiplier for the number of copies of the contained rules
       * we will eventually generate.
       */
      for (const Quantifier &q : n.quantifiers) {
        assert(q.constant() && "non-constant quantifier in ruleset");

        multiplier *= q.count();
      }

      // Descend into our children; we can ignore the quantifiers themselves.
      for (const Ptr<Rule> &r : n.rules)
        dispatch(*r);

      /* Reduce the multiplier, removing our effect. This juggling is necessary
       * because we ourselves may be within a ruleset or contain further
       * rulesets.
       */
      for (const Quantifier &q : n.quantifiers) {
        assert(multiplier % q.count() == 0 &&
               "logic error in handling LivenessCounter::multiplier");

        multiplier /= q.count();
      }
    }

    void visit_propertyrule(const PropertyRule &n) final {
      if (n.property.category == Property::LIVENESS)
        count += multiplier;
      // No need to descend into child nodes.
    }

    virtual ~LivenessCounter() = default;
  };

  // Use the traversal to count liveness rules.
  LivenessCounter lc;
  lc.dispatch(*this);

  return lc.count;
}

void Model::reindex() {
  // Re-number our and our children's 'unique_id' members
  Indexer i;
  i.dispatch(*this);
}

} // namespace rumur
