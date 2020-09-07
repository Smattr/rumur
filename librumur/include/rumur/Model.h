#pragma once

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Function.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <vector>

namespace rumur {

struct Model : public Node {

  // declarations, functions and rules in the order in which they appeared in
  // the source
  std::vector<Ptr<Node>> children;

  __attribute__((deprecated("the 4-argument constructor of Model is "
    "deprecated; please use the newer 2-argument constructor")))
  Model(const std::vector<Ptr<Decl>> &decls_,
    const std::vector<Ptr<Function>> &functions_,
    const std::vector<Ptr<Rule>> &rules_, const location &loc_);

  Model(const std::vector<Ptr<Node>> &children_, const location &loc_);
  virtual ~Model() = default;
  Model *clone() const final;

  // Get the size of the state data in bits.
  mpz_class size_bits() const;

  void validate() const final;

  // dispatch to the appropriate traversal method (see traverse.h)
  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  /* Get the number of global liveness properties in the model. Unlike
   * assumption_count, this considers the "flat" model. That is, a
   * ruleset-contained liveness property may count for more than one.
   */
  mpz_class liveness_count() const;

  /* Update the bit offset of each variable declaration in the model and reindex
   * all AST nodes.
   */
  void reindex();
};

}
