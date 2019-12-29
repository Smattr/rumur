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

  std::vector<Ptr<Decl>> decls;
  std::vector<Ptr<Function>> functions;
  std::vector<Ptr<Rule>> rules;

  Model(const std::vector<Ptr<Decl>> &decls_,
    const std::vector<Ptr<Function>> &functions_,
    const std::vector<Ptr<Rule>> &rules_, const location &loc_);
  virtual ~Model() = default;
  Model *clone() const final;

  // Get the size of the state data in bits.
  mpz_class size_bits() const;

  __attribute__((deprecated("operator== will be removed in a future release")))
  bool operator==(const Node &other) const final;
  void validate() const final;

  // DEPRECATED, DO NOT USE
  __attribute__((deprecated("Model::assumption_count() will be removed in a future release")))
  unsigned long assumption_count() const;

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
