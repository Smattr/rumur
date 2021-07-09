#pragma once

#include <climits>
#include <cstddef>
#include "location.hh"
#include <iostream>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

class BaseTraversal;
class ConstBaseTraversal;

struct RUMUR_API_WITH_RTTI Node {

  location loc;
  size_t unique_id = SIZE_MAX;

  Node() = delete;
  Node(const location &loc_);
  virtual ~Node() = default;

  virtual Node *clone() const = 0;

  /* Confirm that data structure invariants hold. This function throws
   * rumur::Errors if invariants are violated.
   */
  virtual void validate() const {}

  virtual void visit(BaseTraversal &visitor) = 0;
  virtual void visit(ConstBaseTraversal &visitor) const = 0;

protected:
  Node(const Node&) = default;
  Node &operator=(const Node &) = default;
};

} // namespace rumur
