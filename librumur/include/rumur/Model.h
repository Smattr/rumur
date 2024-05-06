#pragma once

#include "location.hh"
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Function.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <vector>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

struct RUMUR_API_WITH_RTTI Model : public Node {

  // declarations, functions and rules in the order in which they appeared in
  // the source
  std::vector<Ptr<Node>> children;

  Model(const std::vector<Ptr<Node>> &children_, const location &loc_);
  virtual ~Model() = default;
  Model *clone() const override;

  // Get the size of the state data in bits.
  mpz_class size_bits() const;

  void validate() const override;

  // dispatch to the appropriate traversal method (see traverse.h)
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;

  /* Get the number of global liveness properties in the model. This considers
   * the “flat” model. That is, a ruleset-contained liveness property may count
   * for more than one.
   */
  mpz_class liveness_count() const;

  /* Update the bit offset of each variable declaration in the model and reindex
   * all AST nodes.
   */
  void reindex();
};

} // namespace rumur
