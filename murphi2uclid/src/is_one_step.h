#pragma once

#include <cstddef>
#include <rumur/rumur.h>

// is the given parameter, representing a for step, known to be 1?
static inline bool is_one_step(const rumur::Ptr<rumur::Expr> &step) {
  if (step == nullptr)
    return true;
  if (!step->constant())
    return false;
  return step->constant_fold() == 1;
}
