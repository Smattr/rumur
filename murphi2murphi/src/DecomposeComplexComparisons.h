// support for turning ==/!= between complex types into a string of comparisons
// on their components

#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include "Stage.h"

class DecomposeComplexComparisons : public IntermediateStage {

 public:
  explicit DecomposeComplexComparisons(Stage &next_);

  void visit_eq(const rumur::Eq &n) final;
  void visit_neq(const rumur::Neq &n) final;

  virtual ~DecomposeComplexComparisons() = default;

 private:
  void rewrite(const rumur::EquatableBinaryExpr &n, bool is_eq);
};
