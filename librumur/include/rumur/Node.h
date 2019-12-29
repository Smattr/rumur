#pragma once

#include <climits>
#include <cstddef>
#include <iostream>
#include "location.hh"

namespace rumur {

struct Node {

  location loc;
  size_t unique_id = SIZE_MAX;

  Node() = delete;
  Node(const location &loc_);
  virtual ~Node() = default;

  virtual Node *clone() const = 0;

  __attribute__((deprecated("operator== will be removed in a future release")))
  virtual bool operator==(const Node &other) const = 0;
  __attribute__((deprecated("operator!= will be removed in a future release")))
  bool operator!=(const Node &other) const {
    return !(*this == other);
  }

  /* Confirm that data structure invariants hold. This function throws
   * rumur::Errors if invariants are violated.
   */
  virtual void validate() const { }

};

}
