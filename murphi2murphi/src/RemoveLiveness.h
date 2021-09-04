// a stage to delete liveness properties from the model

#pragma once

#include "Stage.h"
#include <cstddef>
#include <queue>
#include <rumur/rumur.h>

class RemoveLiveness : public IntermediateStage {

private:
  // does the next seen semi-colon need to be deleted?
  bool swallow_semi = false;

  // queued updates to .swallow_semi
  std::queue<bool> state;

public:
  explicit RemoveLiveness(Stage &next_);

  // interpose on the output, so we have a chance to suppress any spurious
  // semi-colons following a deleted liveness property
  void process(const Token &t) final;

  // Structurally a liveness property can be contained within a PropertyRule or
  // a PropertyStmt. However, liveness properties are only legally allowed to
  // exist as top-level claims. Hence we know that a validated model will never
  // have any liveness properties within PropertyStmts and so we only need to
  // override visit_propertyrule() and not also visit_propertystmt().

  void visit_propertyrule(const rumur::PropertyRule &n) final;
};
